//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module SCALAR.CPP | SCALAR source file for all test modules.
//

#include "modstandard.hpp"

#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

#include "SCALAR.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x61b3ef51, 0xe2e1, 0x11d1, { 0x92, 0xb5, 0x00, 0x60, 0x08, 0x93, 0xa2, 0xb2 }};
DECLARE_MODULE_NAME("Scalar");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Tests ODBC Canonical functions");
DECLARE_MODULE_VERSION(795921705);
// }}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CTable		*g_pTable = NULL;
WCHAR		g_wszScalarFmt[MAX_BUF];

//Check whether the provider is read only.
BOOL	g_fReadOnlyProvider = FALSE;

typedef struct tagGlobalProviderInfo {
	ULONG		ulStringFuncs;
	ULONG		ulNumericFuncs;
	ULONG		ulSystemFuncs;
	ULONG		ulDateTimeFuncs;
	ULONG		ulOJSupport;
	ULONG		ulConvertInteger;
	ULONG		ulConvertTime;
	ULONG		ulConvertTimeStamp;
	ULONG		ulConvertLongVarBinary;
	ULONG		ulConvertLongVarChar;
	ULONG		ulConvertNumeric;
	ULONG		ulConvertReal;
	ULONG		ulConvertSmallint;
	ULONG		ulConvertTinyint; 
	ULONG		ulConvertGuid;
	ULONG		ulConvertWchar;
	ULONG		ulConvertVarchar;
	ULONG		ulConvertDecimal;
	ULONG		ulConvertVarbinary;
	ULONG		ulConvertChar;
	ULONG		ulConvertWLongVarChar;
	ULONG		ulConvertWVarChar;
	ULONG		dwDBMSType;
	ULONG		wszDBMSName[50];
	ULONG		ulVersion;
	} GlobalProviderInfo;

GlobalProviderInfo	g_ProvInfo;

static void SetGlobalProviderInfo(GlobalProviderInfo *pProvInfo, int Provider);

BOOL CanUseScalarFunctions(IDBProperties *pIDBProperties)
{
	ULONG		cPropertyIDSets = 1;
	ULONG		cPropertySets = 0;
	DBPROPSET	*rgPropertySets=NULL;
	DBPROPIDSET rgPropertyIDSets[1];
	DBPROPID	dbPropId[3] = { DBPROP_SQLSUPPORT, DBPROP_DBMSNAME, DBPROP_DBMSVER };
	int			i = 0;
	BOOL		fRet = FALSE;
	static		WCHAR *DBMSTargetList[] = {L"Microsoft SQL Server", L"Oracle", L"Access", 0};

	ASSERT(pIDBProperties);

	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	rgPropertyIDSets[0].cPropertyIDs = NUMELEM(dbPropId);
	rgPropertyIDSets[0].rgPropertyIDs = dbPropId;

	HRESULT hr = pIDBProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropertySets, &rgPropertySets);
	
	if( S_OK == hr )
	{
		if ((DBPROPVAL_SQL_ESCAPECLAUSES & rgPropertySets[0].rgProperties[0].vValue.iVal))		
			fRet = TRUE;

		while ( DBMSTargetList[i] != NULL )
		{
			if ( 0 == _wcsicmp(DBMSTargetList[i], rgPropertySets->rgProperties[1].vValue.bstrVal ) )
			{
				g_ProvInfo.dwDBMSType = i;
				wcsncpy((WCHAR *)g_ProvInfo.wszDBMSName, rgPropertySets->rgProperties[1].vValue.bstrVal, sizeof(g_ProvInfo.wszDBMSName)/sizeof(WCHAR));
				g_ProvInfo.ulVersion = _wtol(rgPropertySets->rgProperties[2].vValue.bstrVal);

				SetGlobalProviderInfo(&g_ProvInfo, i);

				fRet = TRUE;
				goto END;
			}
			i++;
		}
	}

END:
	FreeProperties(&cPropertySets, &rgPropertySets);
	return fRet;
}

static void SetGlobalProviderInfo(GlobalProviderInfo *pProvInfo, int Provider)
{
	pProvInfo->dwDBMSType = Provider;

	switch (Provider)
	{
	case DRVR_SQLSRVR:
		if (pProvInfo->ulVersion <= 6)
		{
			pProvInfo->ulStringFuncs = SQL_FN_STR_CONCAT | SQL_FN_STR_INSERT | SQL_FN_STR_LEFT | SQL_FN_STR_LTRIM | SQL_FN_STR_LENGTH | SQL_FN_STR_LCASE | SQL_FN_STR_REPEAT | SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM | SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE | SQL_FN_STR_ASCII | SQL_FN_STR_CHAR | SQL_FN_STR_DIFFERENCE | SQL_FN_STR_LOCATE_2 | SQL_FN_STR_SOUNDEX | SQL_FN_STR_SPACE | SQL_FN_STR_BIT_LENGTH | SQL_FN_STR_OCTET_LENGTH;
			pProvInfo->ulNumericFuncs = SQL_FN_NUM_ABS | SQL_FN_NUM_ACOS | SQL_FN_NUM_ASIN | SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2 | SQL_FN_NUM_CEILING | SQL_FN_NUM_COS | SQL_FN_NUM_COT | SQL_FN_NUM_EXP | SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_MOD | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT | SQL_FN_NUM_TAN | SQL_FN_NUM_PI | SQL_FN_NUM_RAND | SQL_FN_NUM_DEGREES | SQL_FN_NUM_LOG10 | SQL_FN_NUM_POWER | SQL_FN_NUM_RADIANS | SQL_FN_NUM_ROUND;
			pProvInfo->ulSystemFuncs =  SQL_FN_SYS_USERNAME | SQL_FN_SYS_DBNAME | SQL_FN_SYS_IFNULL;
			pProvInfo->ulDateTimeFuncs = SQL_FN_TD_NOW | SQL_FN_TD_CURDATE | SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK | SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_MONTH | SQL_FN_TD_QUARTER | SQL_FN_TD_WEEK | SQL_FN_TD_YEAR | SQL_FN_TD_CURTIME | SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE | SQL_FN_TD_SECOND | SQL_FN_TD_TIMESTAMPADD | SQL_FN_TD_TIMESTAMPDIFF | SQL_FN_TD_DAYNAME | SQL_FN_TD_MONTHNAME | SQL_FN_TD_CURRENT_DATE | SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_CURRENT_TIMESTAMP | SQL_FN_TD_EXTRACT;
			pProvInfo->ulOJSupport = SQL_OJ_LEFT | SQL_OJ_RIGHT | SQL_OJ_FULL | SQL_OJ_NESTED | SQL_OJ_NOT_ORDERED | SQL_OJ_INNER | SQL_OJ_ALL_COMPARISON_OPS;
			pProvInfo->ulConvertInteger = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT;
			pProvInfo->ulConvertTime = 0;
			pProvInfo->ulConvertTimeStamp = SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_TIMESTAMP;
			pProvInfo->ulConvertLongVarBinary = SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_LONGVARBINARY;
			pProvInfo->ulConvertLongVarChar = SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR;
			pProvInfo->ulConvertNumeric = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT;
			pProvInfo->ulConvertReal = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT;
			pProvInfo->ulConvertSmallint = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT;
			pProvInfo->ulConvertTinyint = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT;
			pProvInfo->ulConvertGuid = 0;
			pProvInfo->ulConvertWchar = 0;
			pProvInfo->ulConvertVarchar = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_TIMESTAMP | SQL_CVT_LONGVARBINARY;
			pProvInfo->ulConvertDecimal = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT;
			pProvInfo->ulConvertVarbinary = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_TINYINT | SQL_CVT_LONGVARBINARY;
			pProvInfo->ulConvertChar = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_TIMESTAMP | SQL_CVT_LONGVARBINARY;
			pProvInfo->ulConvertWVarChar = 0;
			pProvInfo->ulConvertWLongVarChar = 0;
		}
		else
		{
			pProvInfo->ulStringFuncs =  SQL_FN_STR_CONCAT | SQL_FN_STR_INSERT | SQL_FN_STR_LEFT | SQL_FN_STR_LTRIM | SQL_FN_STR_LENGTH | SQL_FN_STR_LOCATE | SQL_FN_STR_LCASE | SQL_FN_STR_REPEAT | SQL_FN_STR_REPLACE | SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM | SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE | SQL_FN_STR_ASCII | SQL_FN_STR_CHAR | SQL_FN_STR_DIFFERENCE | SQL_FN_STR_LOCATE_2 | SQL_FN_STR_SOUNDEX | SQL_FN_STR_SPACE | SQL_FN_STR_BIT_LENGTH | SQL_FN_STR_OCTET_LENGTH;
			pProvInfo->ulNumericFuncs = SQL_FN_NUM_ABS | SQL_FN_NUM_ACOS | SQL_FN_NUM_ASIN | SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2 | SQL_FN_NUM_CEILING | SQL_FN_NUM_COS | SQL_FN_NUM_COT | SQL_FN_NUM_EXP | SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_MOD | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT | SQL_FN_NUM_TAN | SQL_FN_NUM_PI | SQL_FN_NUM_RAND | SQL_FN_NUM_DEGREES | SQL_FN_NUM_LOG10 | SQL_FN_NUM_POWER | SQL_FN_NUM_RADIANS | SQL_FN_NUM_ROUND | SQL_FN_NUM_TRUNCATE;
			pProvInfo->ulSystemFuncs = SQL_FN_SYS_USERNAME | SQL_FN_SYS_DBNAME | SQL_FN_SYS_IFNULL;
			pProvInfo->ulDateTimeFuncs = SQL_FN_TD_NOW | SQL_FN_TD_CURDATE | SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK | SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_MONTH | SQL_FN_TD_QUARTER | SQL_FN_TD_WEEK | SQL_FN_TD_YEAR | SQL_FN_TD_CURTIME | SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE | SQL_FN_TD_SECOND | SQL_FN_TD_TIMESTAMPADD | SQL_FN_TD_TIMESTAMPDIFF | SQL_FN_TD_DAYNAME | SQL_FN_TD_MONTHNAME | SQL_FN_TD_CURRENT_DATE | SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_CURRENT_TIMESTAMP | SQL_FN_TD_EXTRACT;
			pProvInfo->ulOJSupport = SQL_OJ_LEFT | SQL_OJ_RIGHT | SQL_OJ_FULL | SQL_OJ_NESTED | SQL_OJ_NOT_ORDERED | SQL_OJ_INNER | SQL_OJ_ALL_COMPARISON_OPS;
			pProvInfo->ulConvertInteger = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertTime = 0;
			pProvInfo->ulConvertTimeStamp = SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_TIMESTAMP | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertLongVarBinary = SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_LONGVARBINARY;
			pProvInfo->ulConvertLongVarChar = SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertNumeric = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertReal = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertSmallint = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertTinyint = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertGuid = SQL_CVT_GUID | SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR | SQL_CVT_WLONGVARCHAR;
			pProvInfo->ulConvertWchar = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_TIMESTAMP | SQL_CVT_LONGVARBINARY | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertVarchar = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_TIMESTAMP | SQL_CVT_LONGVARBINARY | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertDecimal = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertVarbinary = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_VARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_TINYINT | SQL_CVT_LONGVARBINARY | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertChar = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_TIMESTAMP | SQL_CVT_LONGVARBINARY | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertWLongVarChar = SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR;
			pProvInfo->ulConvertWVarChar = SQL_CVT_CHAR | SQL_CVT_NUMERIC | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR | SQL_CVT_BINARY | SQL_CVT_VARBINARY | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_TIMESTAMP | SQL_CVT_LONGVARBINARY | SQL_CVT_WCHAR | SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR;
		}
		break;
	case DRVR_ORACLE:
		pProvInfo->ulStringFuncs =  SQL_FN_STR_CONCAT | SQL_FN_STR_LEFT | SQL_FN_STR_LTRIM | SQL_FN_STR_LENGTH | SQL_FN_STR_LCASE | SQL_FN_STR_REPLACE | SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM | SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE | SQL_FN_STR_ASCII | SQL_FN_STR_CHAR | SQL_FN_STR_SOUNDEX;
		pProvInfo->ulNumericFuncs = SQL_FN_NUM_ABS | SQL_FN_NUM_CEILING | SQL_FN_NUM_COS | SQL_FN_NUM_EXP | SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_MOD | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT | SQL_FN_NUM_TAN | SQL_FN_NUM_PI | SQL_FN_NUM_LOG10 | SQL_FN_NUM_POWER | SQL_FN_NUM_ROUND | SQL_FN_NUM_TRUNCATE;
		pProvInfo->ulSystemFuncs =  SQL_FN_SYS_USERNAME | SQL_FN_SYS_IFNULL;
		pProvInfo->ulDateTimeFuncs = SQL_FN_TD_NOW | SQL_FN_TD_CURDATE | SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK | SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_MONTH | SQL_FN_TD_QUARTER | SQL_FN_TD_WEEK | SQL_FN_TD_YEAR | SQL_FN_TD_CURTIME | SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE | SQL_FN_TD_SECOND | SQL_FN_TD_DAYNAME | SQL_FN_TD_MONTHNAME;
		pProvInfo->ulOJSupport = SQL_OJ_LEFT | SQL_OJ_RIGHT | SQL_OJ_NOT_ORDERED;
		pProvInfo->ulConvertInteger = SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_BIGINT;
		pProvInfo->ulConvertTime = SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_DATE | SQL_CVT_TIME | SQL_CVT_TIMESTAMP;
		pProvInfo->ulConvertTimeStamp = SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_DATE; // get rid of time/timestamp due to won't fix limitations
		pProvInfo->ulConvertLongVarBinary = 0;
		pProvInfo->ulConvertLongVarChar = 0;
		pProvInfo->ulConvertNumeric = 0;
		pProvInfo->ulConvertReal = SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_BIGINT;
		pProvInfo->ulConvertSmallint = SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_BIGINT;
		pProvInfo->ulConvertTinyint = SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_BIGINT;
		pProvInfo->ulConvertGuid = 0;
		pProvInfo->ulConvertWchar = 0;
		pProvInfo->ulConvertVarchar = SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_BIGINT | SQL_CVT_DATE | SQL_CVT_TIME | SQL_CVT_TIMESTAMP;
		pProvInfo->ulConvertDecimal = SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_BIGINT;
		pProvInfo->ulConvertVarbinary = 0;
		pProvInfo->ulConvertChar = SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_BIT | SQL_CVT_TINYINT | SQL_CVT_BIGINT | SQL_CVT_DATE | SQL_CVT_TIME | SQL_CVT_TIMESTAMP;
		pProvInfo->ulConvertWLongVarChar = 0;
		pProvInfo->ulConvertWVarChar = 0;
		break;
	case DRVR_QJET:
		pProvInfo->ulStringFuncs = SQL_FN_STR_CONCAT | SQL_FN_STR_LEFT | SQL_FN_STR_LTRIM | SQL_FN_STR_LENGTH | SQL_FN_STR_LOCATE | SQL_FN_STR_LCASE | SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM | SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE | SQL_FN_STR_ASCII | SQL_FN_STR_CHAR | SQL_FN_STR_LOCATE_2 | SQL_FN_STR_SPACE;
		pProvInfo->ulNumericFuncs = SQL_FN_NUM_ABS | SQL_FN_NUM_ATAN | SQL_FN_NUM_CEILING | SQL_FN_NUM_COS | SQL_FN_NUM_EXP | SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_MOD | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT | SQL_FN_NUM_TAN | SQL_FN_NUM_RAND | SQL_FN_NUM_POWER;
		pProvInfo->ulSystemFuncs = 0;
		pProvInfo->ulDateTimeFuncs = SQL_FN_TD_NOW | SQL_FN_TD_CURDATE | SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK | SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_MONTH | SQL_FN_TD_QUARTER | SQL_FN_TD_WEEK | SQL_FN_TD_YEAR | SQL_FN_TD_CURTIME | SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE | SQL_FN_TD_SECOND | SQL_FN_TD_DAYNAME | SQL_FN_TD_MONTHNAME;
		pProvInfo->ulOJSupport = SQL_OJ_LEFT | SQL_OJ_RIGHT | SQL_OJ_NOT_ORDERED | SQL_OJ_ALL_COMPARISON_OPS;
		pProvInfo->ulConvertInteger =  SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertTime = 0;
		pProvInfo->ulConvertTimeStamp = 0;
		pProvInfo->ulConvertLongVarBinary = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertLongVarChar = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertNumeric = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertReal = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertSmallint = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertTinyint = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertGuid = SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertWchar = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertVarchar = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertDecimal = 0;
		pProvInfo->ulConvertVarbinary = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertChar = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertWLongVarChar = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		pProvInfo->ulConvertWVarChar = SQL_CVT_NUMERIC | SQL_CVT_INTEGER | SQL_CVT_SMALLINT | SQL_CVT_FLOAT | SQL_CVT_REAL | SQL_CVT_DOUBLE | SQL_CVT_VARCHAR | SQL_CVT_WVARCHAR;
		break;
	default:
		ASSERT(!"No Support for this configuration\n");
		break;
	}
}


WCHAR * GetidsString(WCHAR *wszBuf, UDWORD id)
{
	for ( int i = 0; i < (sizeof(pwszStringList)/sizeof(pwszStringList[0])); i ++ )
	{
		if (pwszStringList[i].id == id)
		{
			wcscpy(wszBuf, pwszStringList[i].wszItem);
			return wszBuf;
		}
	}

	ASSERT(!L"Bad id passed to GetidsString");
	return NULL;
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
	HRESULT				hr = E_FAIL;
	IDBCreateCommand	*pIDBCreateCommand = NULL;
	IDBProperties		*pIDBProperties = NULL;

	// Get connection and session objects
	if (ModuleCreateDBSession(pThisTestModule))
	{
		// IDBCreateCommand
		if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, 
								SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
		{
			odtLog << L"Commands are not supported." << ENDL;
			return TEST_SKIPPED;
		}

		//Release the pointer
		SAFE_RELEASE(pIDBCreateCommand);
		
		pThisTestModule->m_pIUnknown->QueryInterface(IID_IDBProperties, (void **)&pIDBProperties);
		
		if ( !CanUseScalarFunctions(pIDBProperties) )
		{
			odtLog << L"Can't use scalar functions against this DBMS!" << ENDL;
			return TEST_SKIPPED;
		}
		SAFE_RELEASE(pIDBProperties);		
        
        // Check whether provider is read only
		g_fReadOnlyProvider = IsProviderReadOnly((IUnknown *)pThisTestModule->m_pIUnknown2);

        // This test doesn't support using an ini file, make sure we're not 
		if(GetModInfo()->GetFileName())
		{
			odtLog << L"WARNING: Test does not support using fixed table from ini file.\n";

			// Read only providers must use ini file, so don't reset
			if (!g_fReadOnlyProvider)
			{
				odtLog << L"\tResetting to ignore ini file and use tables created by test.\n";
				GetModInfo()->ResetIniFile();
			}
		}

		g_pTable = new CTable(pThisTestModule->m_pIUnknown2, 0);
		if (!g_pTable || !SUCCEEDED(g_pTable->CreateTable(20)) )
			return FALSE;

		GetidsString(g_wszScalarFmt,idsSelect);
		
		WCHAR wszTmpBuf[2000], wszTmpBuf2[2000];
		wcscat(g_wszScalarFmt,GetidsString(wszTmpBuf,idsCanonShort));
		swprintf(wszTmpBuf,GetidsString(wszTmpBuf2,idsFromString), g_pTable->GetTableName());
		wcscat(g_wszScalarFmt,wszTmpBuf);

		// If we made it this far, everything has succeeded
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
	if (g_pTable)
	{
		g_pTable->DropTable();
		delete g_pTable;
		g_pTable=NULL;
	}

	return ModuleReleaseDBSession(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class CScalar
// @base public | CCommandObject
//
class CScalar : public CCommandObject{
public:
	
	// @cmember ICommand ptr object, around for lifetime of test case
	ICommand *		m_pICommand;
	// @cmember ICommandText Interface pointer
	ICommandText *	m_pICommandText;
	// @cmember ICommandPrepare Interface pointer
	ICommandPrepare *	m_pICommandPrepare;
	// @cmember IRowset Interface pointer
	IRowset *	m_pIRowset;
	// @cmember IAccessor(from IRowset) Interface pointer
	IAccessor *	m_pIRowsetAccessor;

	WCHAR 			m_wszQuery[MAX_BUF];

	WCHAR			m_wszColName[MAX_BUF];

	WCHAR			m_wsz1TypeTableFmt[MAX_BUF];

	// data buffer for parameter data
	BYTE 			m_pData[MAX_BUF];

	// tmp buffer of misc use
	WCHAR 			m_buf[MAX_BUF];
	
	DBBINDING		m_Binding;

	// HACCESSOR
	HACCESSOR		m_hAccessor;

	CTable *		m_pAutoMakeTable;

	CTable *		m_p1TypeTable;


	// @cmember Constructor
	CScalar(LPWSTR wszTestCaseName) : CCommandObject (wszTestCaseName)
	{
		m_pICommandText				= NULL;
		m_pICommand					= NULL;				
		m_pICommandPrepare			= NULL;							
		m_pIRowset					= NULL;				
		m_pIRowsetAccessor			= NULL;	
		m_pAutoMakeTable			= NULL;
		m_p1TypeTable				= NULL;
	};

	// @cmember Constructor
	~CScalar()
	{
		ASSERT(!m_pAutoMakeTable);
		ASSERT(!m_p1TypeTable);
		ASSERT(!m_pICommandText);
		ASSERT(!m_pICommandPrepare);	
		ASSERT(!m_pICommand);				
		ASSERT(!m_pIRowset);				
		ASSERT(!m_pIRowsetAccessor);		
	};

	//@cmember Init
	BOOL Init();
	//@cmember Terminate
	BOOL Terminate();

	// Create a binding to ordinal one
	HRESULT	CScalar::CreateABinding();

	// Release rowset and accesor interfaces
	HRESULT CScalar::ReleaseRowset();

	// Open a rowset, fetch the first row, and get the data
	HRESULT CScalar::ExecuteQuery(WCHAR *wszQuery, BOOL fPrepare=NO_PREPARE);

	// Execute a scalar function and verified result against ExpBuf
	BOOL CScalar::CheckQry(WCHAR *ExpBuf, int compType);

	// Help function to verify Results
	BOOL CScalar::CheckStr(size_t iPrec, WCHAR *wszBuf,WCHAR *wszExpBuf, int compType);

	// Determines whether a particular string function is supported
	BOOL CScalar::IsSupportedScalar(DWORD dwBitMask);

	// Determines whether a particular numeric function is supported
	BOOL CScalar::IsSupportedNumeric(DWORD dwBitMask);

	// Determines whether a particular date time function is supported
	BOOL CScalar::IsSupportedDateTime(DWORD dwBitMask);

	// Determines whether a particular System function is supported
	BOOL CScalar::IsSupportedSystem(DWORD dwBitMask);

	// Determines whether a particular Outer Join capability is supported
	BOOL CScalar::IsSupportedOJ(DWORD dwBitMask);

	// Determines whether a particular conversion via fn convert is supported
	BOOL CScalar::IsSupportedConversion(ULONG ulFromType, ULONG ulToType);

	// Just execute a query and discard the result set
	BOOL ExecuteDirect(WCHAR *wszQuery);

	// Verify Results of {fn convert( , )}
	BOOL CheckConversion(ULONG wFromType, ULONG wToType, WCHAR *wszInput, ULONG ulScalarID, int CompType, WCHAR *wszExpValue = NULL, WCHAR *wszExpBackup = NULL);

	// Create a table with specified ODBC SQL Type
	BOOL CreateTypeTable (int iSqlType);

	// Drop the one type table
	BOOL DropTypeTable();

	// map the ODBC SQL Type to a native type name
	WCHAR * MapSqlToNativeType(int iSqlType);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CScalar::Init()
{
  	if (COLEDB::Init())	
	{
		// Set needed pointers
		SetDataSourceObject(m_pThisTestModule->m_pIUnknown, TRUE);
		SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);

		// Get a Command object
		if (FAILED(m_pIDBCreateCommand->CreateCommand(
						NULL, IID_ICommand, (IUnknown **)&m_pICommand)))
		{
			odtLog << L"Initialization failed!" << ENDL;
			return FALSE;
		}
			
		VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE, (IUnknown **)&m_pIAccessor);
		VerifyInterface(m_pICommand, IID_ICommandText, COMMAND_INTERFACE, (IUnknown **)&m_pICommandText);
		VerifyInterface(m_pICommand, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown **)&m_pICommandPrepare);

		return TRUE;
	}  

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Terminate
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CScalar::Terminate()
{
	// Release objects
	SAFE_RELEASE(m_pICommand);
	SAFE_RELEASE(m_pIAccessor);
	SAFE_RELEASE(m_pICommandText);
	SAFE_RELEASE(m_pICommandPrepare);
	
	ReleaseDBSession();
	ReleaseDataSourceObject();
	
	return(COLEDB::Terminate());
}


HRESULT	CScalar::CreateABinding()
{
	DBBINDSTATUS BindStatus = 0;

	m_Binding.iOrdinal = 1;
	m_Binding.dwPart = DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH;
	m_Binding.eParamIO = 0;	
	m_Binding.pTypeInfo = NULL;
	m_Binding.obValue = offsetof(ValueInfo, pValue);
	m_Binding.cbMaxLen = sizeof(m_pData);
	m_Binding.obLength = offsetof(ValueInfo, cbLength);
	m_Binding.obStatus = offsetof(ValueInfo, dbsStatus);
	m_Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	m_Binding.wType = DBTYPE_WSTR;
	m_Binding.pBindExt = NULL;
	m_Binding.bPrecision = 0;
	m_Binding.bScale = 0;

	return  m_pIRowsetAccessor->CreateAccessor(
								DBACCESSOR_ROWDATA,
								1, 
								&m_Binding,
								0,
								&m_hAccessor,
								&BindStatus);
}

HRESULT CScalar::ReleaseRowset()
{
	if (m_pIRowsetAccessor)
		m_pIRowsetAccessor->ReleaseAccessor(m_hAccessor,NULL);
	SAFE_RELEASE(m_pIRowsetAccessor);
	SAFE_RELEASE(m_pIRowset);

	return S_OK;
}

HRESULT CScalar::ExecuteQuery(WCHAR *wszQuery, BOOL fPrepare)
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		hrow=DB_NULL_HROW;
	HROW *	 	pHRow=&hrow; 


	ASSERT(m_pICommandText);

	if (!CHECK(m_hr=m_pICommandText->SetCommandText(DBGUID_DEFAULT, wszQuery),S_OK))
		return m_hr;

	if (fPrepare)
	{
		if (!m_pICommandPrepare )
			return E_FAIL;
		else
			if (!CHECK(m_hr=m_pICommandPrepare->Prepare(0),S_OK))
				return m_hr;
	}

	if (!CHECK(m_hr=m_pICommandText->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&m_pIRowset), S_OK))
		return m_hr;

	if (!VerifyInterface(m_pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&m_pIRowsetAccessor))
		return E_NOINTERFACE;

	if (!CHECK(m_hr=CreateABinding(),S_OK))
		return m_hr;

	if (!CHECK(m_hr=m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow),S_OK))
		return m_hr;
	
	COMPARE(cRowsObtained, 1);			

	if (!CHECK(m_hr=m_pIRowset->GetData(pHRow[0], m_hAccessor, m_pData),S_OK))
		return m_hr;
	
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	return S_OK;
}


BOOL CScalar::CheckQry(WCHAR *ExpBuf, int compType)
{
	WCHAR	*QueryOut=NULL;
	BOOL	fPass = TEST_PASS;
	IRowset	*pRowset=NULL;
	ULONG	cRowsAffected=0;
	
	if(!CHECK(ExecuteQuery(m_wszQuery),S_OK))
		goto CLEANUP;

	// DebugMsg(buf);
 	if(wcscmp(ExpBuf,L"IGNORE"))
		fPass &= CheckStr(wcslen(ExpBuf),(WCHAR *)((BYTE *)m_pData+m_Binding.obValue), ExpBuf, compType);
	ReleaseRowset();

	//rc=SQLGetFunctions(hdbc0,SQL_API_SQLNATIVESQL,pfExists);		
	//FSSQLNativeSql(hdbc0,m_szQuery,SQL_NTS,QueryOut,MAX_BUF,NULL);
	if (!CHECK(m_pICommandText->GetCommandText(NULL, &QueryOut), S_OK))
		goto CLEANUP;

	//DebugMsg(QueryOut);
	if(!CHECK(ExecuteQuery(QueryOut),S_OK))
		goto CLEANUP;


 	if(wcscmp(ExpBuf,L"IGNORE"))
 		fPass &= CheckStr(wcslen(ExpBuf),(WCHAR *)((BYTE *)m_pData+m_Binding.obValue), ExpBuf, compType);

	PROVIDER_FREE(QueryOut);
	ReleaseRowset();

//	Use Prepare/Execute
	if (!m_pICommandPrepare)
		goto CLEANUP;

	if(!CHECK(ExecuteQuery(m_wszQuery, PREPARE),S_OK))
		goto CLEANUP;
		
	if(wcscmp(ExpBuf,L"IGNORE"))
 		fPass &= CheckStr(wcslen(ExpBuf),(WCHAR *)((BYTE *)m_pData+m_Binding.obValue), ExpBuf,compType);		
	ReleaseRowset();

	//rc=SQLGetFunctions(hdbc0,SQL_API_SQLNATIVESQL,pfExists);
	//if(FSSQLNativeSql(hdbc0,szQuery,SQL_NTS,QueryOut,MAX_BUF,NULL))
	if (!CHECK(m_pICommandText->GetCommandText(NULL, &QueryOut), S_OK))
		goto CLEANUP;
	if(!CHECK(ExecuteQuery(QueryOut, PREPARE),S_OK))
		goto CLEANUP;
	
	if(wcscmp(ExpBuf,L"IGNORE"))
 		fPass &= CheckStr(wcslen(ExpBuf),(WCHAR *)((BYTE *)m_pData+m_Binding.obValue), ExpBuf, compType);
	PROVIDER_FREE(QueryOut);
	ReleaseRowset();

CLEANUP:
	ReleaseRowset();  // Cleanup in case of premature termination
	return fPass;
}


BOOL CScalar::CheckStr(size_t iPrec, WCHAR *wszBuf,WCHAR *wszExpBuf, int compType)
{
	WCHAR *buf=(WCHAR *)PROVIDER_ALLOC(2*MAXBUFLEN*sizeof(WCHAR));
	WCHAR *buf1=(WCHAR *)PROVIDER_ALLOC(2*MAXBUFLEN*sizeof(WCHAR));	
	double ldReturn, ldExpect, maxDiff;
	float	rReturn, rExpect;
	long lReturn, lExpect;
	int errorFlag = FALSE;
	WCHAR	*ss=NULL;

	switch (compType)
	{
	case CHECKSTR:
		if(!COMPARE(0==wcsncmp(wszExpBuf,wszBuf,iPrec),TRUE))
			errorFlag = TRUE;
		break;
	case CHECKISTR:
		if(!COMPARE(0==_wcsicmp(wszExpBuf,wszBuf),TRUE))
			errorFlag = TRUE;
		break;
	case CHECKINT:
		lExpect = _wtol(wszExpBuf);
		lReturn = _wtol(wszBuf);
		if(!COMPARE(lExpect,lReturn))
			errorFlag = TRUE;
		break;
	case CHECKFLOAT:
		ldExpect = wcstod(wszExpBuf,&ss);
		ldReturn = wcstod(wszBuf,&ss);

		maxDiff = (1.0 / pow(2.0,47.0));
		if ( !COMPARE(fabs((double)(ldReturn - ldExpect)) <= (maxDiff), TRUE) )
			errorFlag = TRUE;
		break;
	case CHECKREAL:
		rExpect = float(wcstod(wszExpBuf,&ss));
		rReturn = float(wcstod(wszBuf,&ss));

		maxDiff = (1.0 / pow(2.0,21.0));
		if ( !COMPARE(fabs((double)(rReturn - rExpect)) <= (maxDiff), TRUE) )
			errorFlag = TRUE;
		break;
	case CHECKTIMESTAMP:
		{
		DBTIMESTAMP *ptsExpected = NULL, *ptsActual = NULL;

		ptsExpected = (DBTIMESTAMP *) WSTR2DBTYPE(wszExpBuf, DBTYPE_DBTIMESTAMP, NULL);
		ptsActual = (DBTIMESTAMP *) WSTR2DBTYPE(wszBuf, DBTYPE_DBTIMESTAMP, NULL);

		errorFlag = ((ptsActual->year != ptsExpected->year) ||
					(ptsActual->month != ptsExpected->month) ||
					(ptsActual->day != ptsExpected->day) ||
					(ptsActual->hour != ptsExpected->hour) ||
					(ptsActual->minute != ptsExpected->minute) ||
					(ptsActual->second != ptsExpected->second) ||
					(ptsActual->fraction != ptsExpected->fraction  ));

		PROVIDER_FREE(ptsExpected);
		PROVIDER_FREE(ptsActual);
		break;
		}
	default:
		ASSERT(!L"I must've forgot to change something...");
		break;
	}

 	if(errorFlag)
	{
		odtLog << m_wszQuery << ENDL;
		wcscpy(buf,L"Returned: ");
		wcsncat(buf,wszBuf,MAXBUFLEN);
		odtLog << buf << ENDL;
		wcscpy(buf1,L"Expected: ");
		wcsncat(buf1,wszExpBuf,MAXBUFLEN);
		odtLog << buf1 << ENDL;
	}
	PROVIDER_FREE(buf);
 	PROVIDER_FREE(buf1);
	return !errorFlag;
}

// Determines whether a particular string function is supported
BOOL CScalar::IsSupportedScalar(DWORD dwBitMask)
{
	return g_ProvInfo.ulStringFuncs & dwBitMask;
}

// Determines whether a particular numeric function is supported
BOOL CScalar::IsSupportedNumeric(DWORD dwBitMask)
{
	return g_ProvInfo.ulNumericFuncs & dwBitMask;
}

// Determines whether a particular date time function is supported
BOOL CScalar::IsSupportedDateTime(DWORD dwBitMask)
{
	return g_ProvInfo.ulDateTimeFuncs & dwBitMask;
}

// Determines whether a particular System function is supported
BOOL CScalar::IsSupportedSystem(DWORD dwBitMask)
{
	return g_ProvInfo.ulSystemFuncs & dwBitMask;
}

// Determines whether a particular Outer Join capability is supported
BOOL CScalar::IsSupportedOJ(DWORD dwBitMask)
{
	return g_ProvInfo.ulOJSupport & dwBitMask;
}

// Determines whether a particular conversion via fn convert is supported
BOOL CScalar::IsSupportedConversion(ULONG ulFromType, ULONG ulToType)
{
	switch (ulFromType)
	{
	case SQL_CONVERT_INTEGER:
		return g_ProvInfo.ulConvertInteger & ulToType;

	case SQL_CONVERT_TIME:
		return g_ProvInfo.ulConvertTime & ulToType;

	case SQL_CONVERT_TIMESTAMP:
		return g_ProvInfo.ulConvertTimeStamp & ulToType;

	case SQL_CONVERT_LONGVARCHAR:
		return g_ProvInfo.ulConvertLongVarChar & ulToType;

	case SQL_CONVERT_LONGVARBINARY:
		return g_ProvInfo.ulConvertLongVarBinary & ulToType;

	case SQL_CONVERT_NUMERIC:
		return g_ProvInfo.ulConvertNumeric & ulToType;

	case SQL_CONVERT_REAL:
		return g_ProvInfo.ulConvertReal & ulToType;

	case SQL_CONVERT_SMALLINT:
		return g_ProvInfo.ulConvertSmallint & ulToType;

	case SQL_CONVERT_TINYINT:
		return g_ProvInfo.ulConvertTinyint & ulToType;

	case SQL_CONVERT_GUID:
		return g_ProvInfo.ulConvertGuid & ulToType;

	case SQL_CONVERT_WCHAR:
		return g_ProvInfo.ulConvertWchar & ulToType;

	case SQL_CONVERT_VARCHAR:
		return g_ProvInfo.ulConvertVarchar & ulToType;

	case SQL_CONVERT_VARBINARY:
		return g_ProvInfo.ulConvertVarbinary & ulToType;

	case SQL_CONVERT_DECIMAL:
		return g_ProvInfo.ulConvertDecimal & ulToType;

	case SQL_CONVERT_CHAR:
		return g_ProvInfo.ulConvertChar & ulToType;

	case SQL_CONVERT_WVARCHAR:
		return g_ProvInfo.ulConvertWVarChar & ulToType;
	
	case SQL_CONVERT_WLONGVARCHAR:
		return g_ProvInfo.ulConvertWLongVarChar & ulToType;
	
	default:
		ASSERT(!"No support for this Type!\n");
		return FALSE;
	}
}


// Just execute a query and discard the result set
BOOL CScalar::ExecuteDirect(WCHAR *wszQuery)
{
	if (!CHECK(m_hr=ExecuteQuery(wszQuery, NO_PREPARE), S_OK))
		return FALSE;
	if (!CHECK(m_hr=ReleaseRowset(),S_OK))
		return FALSE;

	return TRUE;
}

// Check the results of {fn convert(,)}
BOOL CScalar::CheckConversion(ULONG ulFromType, ULONG ulToType, WCHAR *wszInputValue, ULONG ulScalarID, int iCheckType, WCHAR *wszExpValue, WCHAR *wszExpBackUp)
{	
	WCHAR		buf[MAX_BUF], wszInputBuf[MAX_BUF];
	BOOL		fPass = TEST_FAIL;
	DBORDINAL	ulColOrdinal = 1;
	CCol		TempCol;

	if(!IsSupportedConversion(ulFromType, ulToType))
		return TEST_SKIPPED;

	// Hacks for Won't Fix server behavior
	if (wszExpValue == NULL)
		wszExpValue = wszInputValue;

	if (wszExpBackUp == NULL)
		wszExpBackUp = wszExpValue;
	// end hack
	
	// format literal value
	wcscpy(wszInputBuf, L"\0");
	m_p1TypeTable->GetColInfo(1, TempCol);
									
	if(TempCol.GetPrefix())
	{
		if (TempCol.GetProviderType() == DBTYPE_DBTIMESTAMP)
			wcscat(wszInputBuf, L"{ ts '");
		else
			wcscat(wszInputBuf,TempCol.GetPrefix());
	}

	wcscat(wszInputBuf,wszInputValue);										
		
	if(TempCol.GetSuffix())
	{
		if (TempCol.GetProviderType() == DBTYPE_DBTIMESTAMP)
			wcscat(wszInputBuf, L"' }");
		else
			wcscat(wszInputBuf, TempCol.GetSuffix());
	}

	// create the simple literal canonical select
	swprintf(buf,GetidsString(m_buf, ulScalarID), wszInputBuf);
	swprintf(m_wszQuery,g_wszScalarFmt,buf);
	
	fPass = CheckQry(wszExpValue, iCheckType);
	if (fPass != TEST_PASS)
		return fPass;  // just end right here

	fPass = TEST_FAIL;

	// The simple select worked, now use column name in place of literal

	// Insert the target value
	if (!CHECK(m_hr=m_p1TypeTable->InsertWithUserLiterals(1, &ulColOrdinal, &wszInputValue),S_OK))
		goto CLEANUP;

	// Execute the query using the column name instead of literal
	swprintf(buf,GetidsString(m_buf, ulScalarID), m_wszColName);
	swprintf(m_wszQuery,m_wsz1TypeTableFmt, buf);

	fPass = CheckQry(wszExpBackUp, iCheckType);

CLEANUP:
	// Delete all values from the table	
	swprintf(m_wszQuery,L"delete from %s", m_p1TypeTable->GetTableName());
	CHECK(m_pICommandText->SetCommandText(DBGUID_DEFAULT, m_wszQuery),S_OK);
	CHECK(m_pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL), S_OK);

	return fPass;
}

// Create a table with specified ODBC SQL Type
BOOL CScalar::CreateTypeTable (int iSqlType)
{
	BOOL	fSuccess = FALSE;
	CList 	<WCHAR * ,WCHAR* > TypesList;
	WCHAR	*wszProviderTypeName = NULL;
	CCol	TempCol;

	// A List of the one type we're interested in.
	wszProviderTypeName = MapSqlToNativeType(iSqlType);
	if (!wszProviderTypeName)
		return TEST_SKIPPED;

	TypesList.AddTail(wszProviderTypeName);

	m_p1TypeTable = new CTable(m_pThisTestModule->m_pIUnknown2, 0);
	if (!m_p1TypeTable || !SUCCEEDED(m_p1TypeTable->CreateTable(TypesList, 0, 0)))
		goto exit01;

	CHECK(m_p1TypeTable->GetColInfo(1, TempCol), S_OK);
	wcscpy(m_wszColName, TempCol.GetColName());
	fSuccess = TRUE;

	wcscpy(m_wsz1TypeTableFmt, L"select {%s} from ");
	wcscat(m_wsz1TypeTableFmt, m_p1TypeTable->GetTableName());
	
exit01:
	TypesList.RemoveAll();
	return fSuccess;
}

// Drop the one type table
BOOL CScalar::DropTypeTable()
{
	if (m_p1TypeTable)
	{
		m_p1TypeTable->DropTable();
		delete m_p1TypeTable;
		m_p1TypeTable = NULL;
	}

	return TRUE;
}

// map the ODBC SQL Type to a native type name
WCHAR * CScalar::MapSqlToNativeType(int iSqlType)
{
	static WCHAR *rgwszNativeOracleTypes[] = {L"FLOAT", L"LONG RAW", L"RAW", L"CHAR", L"LONG", L"VARCHAR2", L"NUMBER(10,0)", L"DATE", L"NUMBER()"};
	static WCHAR *rgwszHydraTypes[] = {L"int", L"datetime"};


	switch (iSqlType)
	{
	case SQL_INTEGER:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:
			return L"int";
		case DRVR_ORACLE:
			return L"number(10,0)";
		case DRVR_QJET:
			return L"integer";
		}
		break;

	case SQL_TIMESTAMP:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:
		case DRVR_QJET:
			return L"datetime";
		case DRVR_ORACLE:
			return L"date";		
		}
		break;

	case SQL_GUID:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:
			if (g_ProvInfo.ulVersion >= 7)
				return L"uniqueidentifier";
			break;
		case DRVR_QJET:
			return L"GUID";
		}
		break;

	case SQL_CHAR:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:
			if (g_ProvInfo.dwDBMSType >= 7)
				return L"char(8000)";
			else
				return L"char(254)";
		case DRVR_ORACLE:
			return L"char(1000)";
		}
		break;

	case SQL_VARCHAR:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:
			if (g_ProvInfo.dwDBMSType >= 7)
				return L"varchar(8000)";
			else
				return L"varchar(254)";
		case DRVR_ORACLE:
			return L"VARCHAR2(2000)";
		}
		break;

	case SQL_LONGVARCHAR:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:		
			return L"text";
		case DRVR_ORACLE:
			return L"long";
		}
		break;

	case SQL_DOUBLE:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:		
			return L"float";
		case DRVR_ORACLE:
			return L"float";
		case DRVR_QJET:
			return L"double";
		}
		break;

	case SQL_REAL:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_QJET:
		case DRVR_SQLSRVR:		
			return L"real";		
		}
		break;

	case SQL_BINARY:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_QJET:
		case DRVR_SQLSRVR:		
			return L"binary(254)";
		}
		break;

	case SQL_VARBINARY:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_QJET:
		case DRVR_SQLSRVR:		
			return L"varbinary(254)";
		case DRVR_ORACLE:
			return L"RAW(254)";
		}
		break;

	case SQL_LONGVARBINARY:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:		
			return L"image";
		case DRVR_ORACLE:
			return L"LONG RAW";
		case DRVR_QJET:
			return L"LONGBINARY";
		}
		break;

	case SQL_DECIMAL:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:		
			return L"decimal(28,28)";
		case DRVR_ORACLE:
			return L"number(38,38)";
//		case DRVR_QJET:
//			return L"decimal (28,28)";
		}
		break;

	case SQL_NUMERIC:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:		
			return L"numeric(28,0)";
		case DRVR_ORACLE:
			return L"number(38,0)";
//		case DRVR_QJET:
//			return L"decimal (28, 0)";
		}
		break;

	case SQL_SMALLINT:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_QJET:
		case DRVR_SQLSRVR:		
			return L"smallint";
		case DRVR_ORACLE:
			return L"number(5,0)";		
		}
		break;

	case SQL_TINYINT:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:		
			return L"tinyint";
		case DRVR_ORACLE:
			return L"number(3,0)";
		case DRVR_QJET:
			return L"BYTE";
		}
		break;

	case SQL_WCHAR:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:
			if (g_ProvInfo.ulVersion >= 7)
				return L"nchar(4000)";
			break;
		case DRVR_QJET:
			return L"char(255)";
		}
		break;

	case SQL_WVARCHAR:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:
			if (g_ProvInfo.ulVersion >= 7)
				return L"nvarchar(8000)";
			break;
		case DRVR_QJET:
			return L"VARCHAR(255)";
		}
		break;

	case SQL_WLONGVARCHAR:
		switch(g_ProvInfo.dwDBMSType)
		{
		case DRVR_SQLSRVR:
			if (g_ProvInfo.ulVersion >= 7)
				return L"ntext";
			break;
		case DRVR_QJET:
			return L"LONGCHAR";
		}
		break;

	case SQL_TIME:
	case SQL_DATE:
		break;

	default:
		ASSERT(!L"No Support for this ODBC SQL TYPE\n");
		break;
	}

	return NULL;  // no such type for this provider
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(StringFunctions)
//--------------------------------------------------------------------
// @class Test scalar string functions
//
class StringFunctions : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(StringFunctions,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ASCII Bz
	int Variation_1();
	// @cmember CHAR 102
	int Variation_2();
	// @cmember CONCAT Fire Truck
	int Variation_3();
	// @cmember LTRIM  a BCDef
	int Variation_4();
	// @cmember LENGTH
	int Variation_5();
	// @cmember LOCATE st in forest estates
	int Variation_6();
	// @cmember LCASE fIrEtRucK
	int Variation_7();
	// @cmember REPEAT tq 3
	int Variation_8();
	// @cmember REPLACE forest estates  es truck
	int Variation_9();
	// @cmember RIGHT forest estates 7
	int Variation_10();
	// @cmember RTRIM  forest estates
	int Variation_11();
	// @cmember SUBSTRING  forest estates 5 4
	int Variation_12();
	// @cmember UCASE forest estates
	int Variation_13();
	// @cmember DIFFERENCE c ab
	int Variation_14();
	// @cmember SOUNDEX ab
	int Variation_15();
	// @cmember SPACE 5
	int Variation_16();
	// @cmember CONCAT nested
	int Variation_17();
	// @cmember LEFT nested
	int Variation_18();
	// @cmember RIGHT nested
	int Variation_19();
	// @cmember LOCATE nested
	int Variation_20();
	// @cmember SUBSTRING nested
	int Variation_21();
	// @cmember DIFFERENCE nested
	int Variation_22();
	// @cmember REPEAT nested
	int Variation_23();
	// @cmember REPLACE nested
	int Variation_24();
	// @cmember INSERT nested
	int Variation_25();
	// @cmember mulit-INSERT nested
	int Variation_26();
	// @cmember Multi CONCAT nested
	int Variation_27();
	// @cmember STRING and NUMERIC
	int Variation_28();
	// }}
};
// {{ TCW_TESTCASE(StringFunctions)
#define THE_CLASS StringFunctions
BEG_TEST_CASE(StringFunctions, CScalar, L"Test scalar string functions")
	TEST_VARIATION(1,		L"ASCII Bz")
	TEST_VARIATION(2,		L"CHAR 102")
	TEST_VARIATION(3,		L"CONCAT Fire Truck")
	TEST_VARIATION(4,		L"LTRIM  a BCDef")
	TEST_VARIATION(5,		L"LENGTH")
	TEST_VARIATION(6,		L"LOCATE st in forest estates")
	TEST_VARIATION(7,		L"LCASE fIrEtRucK")
	TEST_VARIATION(8,		L"REPEAT tq 3")
	TEST_VARIATION(9,		L"REPLACE forest estates  es truck")
	TEST_VARIATION(10,		L"RIGHT forest estates 7")
	TEST_VARIATION(11,		L"RTRIM  forest estates")
	TEST_VARIATION(12,		L"SUBSTRING  forest estates 5 4")
	TEST_VARIATION(13,		L"UCASE forest estates")
	TEST_VARIATION(14,		L"DIFFERENCE c ab")
	TEST_VARIATION(15,		L"SOUNDEX ab")
	TEST_VARIATION(16,		L"SPACE 5")
	TEST_VARIATION(17,		L"CONCAT nested")
	TEST_VARIATION(18,		L"LEFT nested")
	TEST_VARIATION(19,		L"RIGHT nested")
	TEST_VARIATION(20,		L"LOCATE nested")
	TEST_VARIATION(21,		L"SUBSTRING nested")
	TEST_VARIATION(22,		L"DIFFERENCE nested")
	TEST_VARIATION(23,		L"REPEAT nested")
	TEST_VARIATION(24,		L"REPLACE nested")
	TEST_VARIATION(25,		L"INSERT nested")
	TEST_VARIATION(26,		L"mulit-INSERT nested")
	TEST_VARIATION(27,		L"Multi CONCAT nested")
	TEST_VARIATION(28,		L"STRING and NUMERIC")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(NumFunctions)
//--------------------------------------------------------------------
// @class Test numeric scalar functions
//
class NumFunctions : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(NumFunctions,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ABS -123
	int Variation_1();
	// @cmember ACOS 1
	int Variation_2();
	// @cmember ASIN -1
	int Variation_3();
	// @cmember ATAN 2
	int Variation_4();
	// @cmember ATAN2  2  1
	int Variation_5();
	// @cmember CEILING 1.09
	int Variation_6();
	// @cmember COS  0
	int Variation_7();
	// @cmember COT  0.5
	int Variation_8();
	// @cmember EXP   1
	int Variation_9();
	// @cmember FLOOR  1.09
	int Variation_10();
	// @cmember LOG   10
	int Variation_11();
	// @cmember MOD  10  6
	int Variation_12();
	// @cmember PI
	int Variation_13();
	// @cmember RAND  10
	int Variation_14();
	// @cmember SIGN   10
	int Variation_15();
	// @cmember SIN   .05
	int Variation_16();
	// @cmember SQRT   9
	int Variation_17();
	// @cmember TAN   2
	int Variation_18();
	// @cmember DEGREES  3
	int Variation_19();
	// @cmember LOG10    97.1
	int Variation_20();
	// @cmember POWER  9.6   3
	int Variation_21();
	// @cmember RADIANS
	int Variation_22();
	// @cmember ROUND  97.56   1
	int Variation_23();
	// @cmember TRUNCATE  97.56   1
	int Variation_24();
	// }}
};
// {{ TCW_TESTCASE(NumFunctions)
#define THE_CLASS NumFunctions
BEG_TEST_CASE(NumFunctions, CScalar, L"Test numeric scalar functions")
	TEST_VARIATION(1,		L"ABS -123")
	TEST_VARIATION(2,		L"ACOS 1")
	TEST_VARIATION(3,		L"ASIN -1")
	TEST_VARIATION(4,		L"ATAN 2")
	TEST_VARIATION(5,		L"ATAN2  2  1")
	TEST_VARIATION(6,		L"CEILING 1.09")
	TEST_VARIATION(7,		L"COS  0")
	TEST_VARIATION(8,		L"COT  0.5")
	TEST_VARIATION(9,		L"EXP   1")
	TEST_VARIATION(10,		L"FLOOR  1.09")
	TEST_VARIATION(11,		L"LOG   10")
	TEST_VARIATION(12,		L"MOD  10  6")
	TEST_VARIATION(13,		L"PI")
	TEST_VARIATION(14,		L"RAND  10")
	TEST_VARIATION(15,		L"SIGN   10")
	TEST_VARIATION(16,		L"SIN   .05")
	TEST_VARIATION(17,		L"SQRT   9")
	TEST_VARIATION(18,		L"TAN   2")
	TEST_VARIATION(19,		L"DEGREES  3")
	TEST_VARIATION(20,		L"LOG10    97.1")
	TEST_VARIATION(21,		L"POWER  9.6   3")
	TEST_VARIATION(22,		L"RADIANS")
	TEST_VARIATION(23,		L"ROUND  97.56   1")
	TEST_VARIATION(24,		L"TRUNCATE  97.56   1")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DateTimeFn)
//--------------------------------------------------------------------
// @class Test date time scalar functions
//
class DateTimeFn : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DateTimeFn,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember NOW
	int Variation_1();
	// @cmember CURDATE
	int Variation_2();
	// @cmember DAYOFMONTH  1991 02 26
	int Variation_3();
	// @cmember DATOFWEEK  1991 02 26
	int Variation_4();
	// @cmember DAYOFYEAR 1991 02 26
	int Variation_5();
	// @cmember MONTH  1991 02 26
	int Variation_6();
	// @cmember QUARTER 1991 02 26
	int Variation_7();
	// @cmember WEEK  1991 02 26
	int Variation_8();
	// @cmember YEAR 1991 02 26
	int Variation_9();
	// @cmember CURTIME
	int Variation_10();
	// @cmember HOUR 14:49:19
	int Variation_11();
	// @cmember MINUTE  14:49:19
	int Variation_12();
	// @cmember SECOND  14:49:19
	int Variation_13();
	// @cmember DAYNAME d 1994 12 12
	int Variation_14();
	// @cmember MONTHNAME d 1994 12 12
	int Variation_15();
	// @cmember TIMESTAMPADD SQL_TSI_DAY  d 1994 12 12
	int Variation_16();
	// @cmember TIMESTAMPDIFF SQL_TSI_MONTH 11 15 94  12 12 94
	int Variation_17();
	// @cmember CURRENT_TIME
	int Variation_18();
	// @cmember CURRENT_TIME()
	int Variation_19();
	// @cmember CURRENT_TIMESTAMP
	int Variation_20();
	// @cmember CURRENT_TIMESTAMP()
	int Variation_21();
	// }}
};
// {{ TCW_TESTCASE(DateTimeFn)
#define THE_CLASS DateTimeFn
BEG_TEST_CASE(DateTimeFn, CScalar, L"Test date time scalar functions")
	TEST_VARIATION(1,		L"NOW")
	TEST_VARIATION(2,		L"CURDATE")
	TEST_VARIATION(3,		L"DAYOFMONTH  1991 02 26")
	TEST_VARIATION(4,		L"DATOFWEEK  1991 02 26")
	TEST_VARIATION(5,		L"DAYOFYEAR 1991 02 26")
	TEST_VARIATION(6,		L"MONTH  1991 02 26")
	TEST_VARIATION(7,		L"QUARTER 1991 02 26")
	TEST_VARIATION(8,		L"WEEK  1991 02 26")
	TEST_VARIATION(9,		L"YEAR 1991 02 26")
	TEST_VARIATION(10,		L"CURTIME")
	TEST_VARIATION(11,		L"HOUR 14:49:19")
	TEST_VARIATION(12,		L"MINUTE  14:49:19")
	TEST_VARIATION(13,		L"SECOND  14:49:19")
	TEST_VARIATION(14,		L"DAYNAME d 1994 12 12")
	TEST_VARIATION(15,		L"MONTHNAME d 1994 12 12")
	TEST_VARIATION(16,		L"TIMESTAMPADD SQL_TSI_DAY  d 1994 12 12")
	TEST_VARIATION(17,		L"TIMESTAMPDIFF SQL_TSI_MONTH 11 15 94  12 12 94")
	TEST_VARIATION(18,		L"CURRENT_TIME")
	TEST_VARIATION(19,		L"CURRENT_TIME()")
	TEST_VARIATION(20,		L"CURRENT_TIMESTAMP")
	TEST_VARIATION(21,		L"CURRENT_TIMESTAMP()")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(SystemFunctions)
//--------------------------------------------------------------------
// @class Test system scalar functions
//
class SystemFunctions : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SystemFunctions,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember USER
	int Variation_1();
	// @cmember DATABASE
	int Variation_2();
	// @cmember IFNULL NULL  4
	int Variation_3();
	// @cmember IFNULL   -1  4
	int Variation_4();
	// }}
};
// {{ TCW_TESTCASE(SystemFunctions)
#define THE_CLASS SystemFunctions
BEG_TEST_CASE(SystemFunctions, CScalar, L"Test system scalar functions")
	TEST_VARIATION(1,		L"USER")
	TEST_VARIATION(2,		L"DATABASE")
	TEST_VARIATION(3,		L"IFNULL NULL  4")
	TEST_VARIATION(4,		L"IFNULL   -1  4")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(OuterJoins)
//--------------------------------------------------------------------
// @class Test outer join canonical
//
class OuterJoins : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	CTable *	m_pTable2;
	CTable *	m_pTable3;

	WCHAR *		m_wszGlobalTableColName;
	WCHAR *		m_wszTable2ColName;
	WCHAR *		m_wszTable3ColName;

	DBORDINAL	m_ulNumCol;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(OuterJoins,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Left OJ
	int Variation_1();
	// @cmember left OJ   3 Tables
	int Variation_2();
	// @cmember Left OJ   OJ_INNER  3 Tables
	int Variation_3();
	// @cmember Left OJ and OJ_ALL_COMPARISION_OPS
	int Variation_4();
	// @cmember Left OJ and SQL_OJ_NOT_ORDERED
	int Variation_5();
	// @cmember Right OJ
	int Variation_6();
	// @cmember Right OJ and OJ_ALL_COMPARISION_OPS
	int Variation_7();
	// @cmember Right OJ and SQL_OJ_NOT_ORDERED
	int Variation_8();
	// @cmember Nested Outer Join
	int Variation_9();
	// @cmember Nested OJ and OJ_ALL_COMPARISION_OPS
	int Variation_10();
	// @cmember Left OJ with multiple join predicates
	int Variation_11();
	// @cmember Right OJ with multiple join predicates
	int Variation_12();
	// @cmember Left OJ with WHERE clause
	int Variation_13();
	// }}
};
// {{ TCW_TESTCASE(OuterJoins)
#define THE_CLASS OuterJoins
BEG_TEST_CASE(OuterJoins, CScalar, L"Test outer join canonical")
	TEST_VARIATION(1,		L"Left OJ")
	TEST_VARIATION(2,		L"left OJ   3 Tables")
	TEST_VARIATION(3,		L"Left OJ   OJ_INNER  3 Tables")
	TEST_VARIATION(4,		L"Left OJ and OJ_ALL_COMPARISION_OPS")
	TEST_VARIATION(5,		L"Left OJ and SQL_OJ_NOT_ORDERED")
	TEST_VARIATION(6,		L"Right OJ")
	TEST_VARIATION(7,		L"Right OJ and OJ_ALL_COMPARISION_OPS")
	TEST_VARIATION(8,		L"Right OJ and SQL_OJ_NOT_ORDERED")
	TEST_VARIATION(9,		L"Nested Outer Join")
	TEST_VARIATION(10,		L"Nested OJ and OJ_ALL_COMPARISION_OPS")
	TEST_VARIATION(11,		L"Left OJ with multiple join predicates")
	TEST_VARIATION(12,		L"Right OJ with multiple join predicates")
	TEST_VARIATION(13,		L"Left OJ with WHERE clause")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LikeEscape)
//--------------------------------------------------------------------
// @class Test Like Escape clause
//
class LikeEscape : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	CTable	*m_pInsertTable;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LikeEscape,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Canonical Escape
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(LikeEscape)
#define THE_CLASS LikeEscape
BEG_TEST_CASE(LikeEscape, CScalar, L"Test Like Escape clause")
	TEST_VARIATION(1,		L"Canonical Escape")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LiteralDateTimeClauses)
//--------------------------------------------------------------------
// @class Test literal date time escape clauses
//
class LiteralDateTimeClauses : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LiteralDateTimeClauses,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember TimeStamp
	int Variation_1();
	// @cmember Date
	int Variation_2();
	// @cmember Time
	int Variation_3();
	// @cmember INTERVAL YEAR
	int Variation_4();
	// @cmember INTERVAL MONTH
	int Variation_5();
	// @cmember INTERVAL DAY
	int Variation_6();
	// @cmember INTERVAL HOUR
	int Variation_7();
	// @cmember INTERVAL MINUTE
	int Variation_8();
	// @cmember INTERVAL SECOND 3 2
	int Variation_9();
	// @cmember INTERVAL YEAR TO MONTH
	int Variation_10();
	// @cmember INTERVAL DAY TO HOUR
	int Variation_11();
	// @cmember INTERVAL DAY TO MINUTE
	int Variation_12();
	// @cmember INTERVAL  DAY TO SECOND
	int Variation_13();
	// @cmember INTERVAL  HOUR TO MINUTE
	int Variation_14();
	// @cmember INTERVAL  HOUR TO SECOND
	int Variation_15();
	// @cmember INTERVAL  MINUTE TO SECOND
	int Variation_16();
	// @cmember INTERVAL  DAY TO SECOND
	int Variation_17();
	// @cmember GUID
	int Variation_18();
	// }}
};
// {{ TCW_TESTCASE(LiteralDateTimeClauses)
#define THE_CLASS LiteralDateTimeClauses
BEG_TEST_CASE(LiteralDateTimeClauses, CScalar, L"Test literal date time escape clauses")
	TEST_VARIATION(1,		L"TimeStamp")
	TEST_VARIATION(2,		L"Date")
	TEST_VARIATION(3,		L"Time")
	TEST_VARIATION(4,		L"INTERVAL YEAR")
	TEST_VARIATION(5,		L"INTERVAL MONTH")
	TEST_VARIATION(6,		L"INTERVAL DAY")
	TEST_VARIATION(7,		L"INTERVAL HOUR")
	TEST_VARIATION(8,		L"INTERVAL MINUTE")
	TEST_VARIATION(9,		L"INTERVAL SECOND 3 2")
	TEST_VARIATION(10,		L"INTERVAL YEAR TO MONTH")
	TEST_VARIATION(11,		L"INTERVAL DAY TO HOUR")
	TEST_VARIATION(12,		L"INTERVAL DAY TO MINUTE")
	TEST_VARIATION(13,		L"INTERVAL  DAY TO SECOND")
	TEST_VARIATION(14,		L"INTERVAL  HOUR TO MINUTE")
	TEST_VARIATION(15,		L"INTERVAL  HOUR TO SECOND")
	TEST_VARIATION(16,		L"INTERVAL  MINUTE TO SECOND")
	TEST_VARIATION(17,		L"INTERVAL  DAY TO SECOND")
	TEST_VARIATION(18,		L"GUID")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvertClause)
//--------------------------------------------------------------------
// @class Test canonical function convert
//
class ConvertClause : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvertClause,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Integer to SQL_BIGINT
	int Variation_1();
	// @cmember Integer to SQL_BINARY
	int Variation_2();
	// @cmember Integer to SQL_BIT
	int Variation_3();
	// @cmember Integer to SQL_CHAR
	int Variation_4();
	// @cmember Integer to SQL_DATE
	int Variation_5();
	// @cmember Integer to SQL_DECIMAL
	int Variation_6();
	// @cmember Integer to SQL_DOUBLE
	int Variation_7();
	// @cmember Integer to SQL_FLOAT
	int Variation_8();
	// @cmember Integer to SQL_LONGVARBINARY
	int Variation_9();
	// @cmember Integer to SQL_LONGVARCHAR
	int Variation_10();
	// @cmember Integer to SQL_NUMERIC
	int Variation_11();
	// @cmember Integer to SQL_REAL
	int Variation_12();
	// @cmember Integer to SQL_SMALLINT
	int Variation_13();
	// @cmember Integer to SQL_TIME
	int Variation_14();
	// @cmember Integer to SQL_TIMESTAMP
	int Variation_15();
	// @cmember Integer to SQL_TINYINT
	int Variation_16();
	// @cmember Integer to SQL_VARBINARY
	int Variation_17();
	// @cmember Integer to SQL_VARCHAR
	int Variation_18();
	// @cmember To SQL_WCHAR
	int Variation_19();
	// @cmember To SQL_VARWCHAR
	int Variation_20();
	// @cmember To SQL_WLONGVARCHAR
	int Variation_21();
	// }}
};
// {{ TCW_TESTCASE(ConvertClause)
#define THE_CLASS ConvertClause
BEG_TEST_CASE(ConvertClause, CScalar, L"Test canonical function convert")
	TEST_VARIATION(1,		L"Integer to SQL_BIGINT")
	TEST_VARIATION(2,		L"Integer to SQL_BINARY")
	TEST_VARIATION(3,		L"Integer to SQL_BIT")
	TEST_VARIATION(4,		L"Integer to SQL_CHAR")
	TEST_VARIATION(5,		L"Integer to SQL_DATE")
	TEST_VARIATION(6,		L"Integer to SQL_DECIMAL")
	TEST_VARIATION(7,		L"Integer to SQL_DOUBLE")
	TEST_VARIATION(8,		L"Integer to SQL_FLOAT")
	TEST_VARIATION(9,		L"Integer to SQL_LONGVARBINARY")
	TEST_VARIATION(10,		L"Integer to SQL_LONGVARCHAR")
	TEST_VARIATION(11,		L"Integer to SQL_NUMERIC")
	TEST_VARIATION(12,		L"Integer to SQL_REAL")
	TEST_VARIATION(13,		L"Integer to SQL_SMALLINT")
	TEST_VARIATION(14,		L"Integer to SQL_TIME")
	TEST_VARIATION(15,		L"Integer to SQL_TIMESTAMP")
	TEST_VARIATION(16,		L"Integer to SQL_TINYINT")
	TEST_VARIATION(17,		L"Integer to SQL_VARBINARY")
	TEST_VARIATION(18,		L"Integer to SQL_VARCHAR")
	TEST_VARIATION(19,		L"To SQL_WCHAR")
	TEST_VARIATION(20,		L"To SQL_VARWCHAR")
	TEST_VARIATION(21,		L"To SQL_WLONGVARCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvertFromTim)
//--------------------------------------------------------------------
// @class Test conversion from TIME
//
class ConvertFromTim : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvertFromTim,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember TO SQL_CHAR
	int Variation_1();
	// @cmember TO SQL_DATE
	int Variation_2();
	// @cmember To SQL_TIME
	int Variation_3();
	// @cmember To SQL_TIMESTAMP
	int Variation_4();
	// }}
};
// {{ TCW_TESTCASE(ConvertFromTim)
#define THE_CLASS ConvertFromTim
BEG_TEST_CASE(ConvertFromTim, CScalar, L"Test conversion from TIME")
	TEST_VARIATION(1,		L"TO SQL_CHAR")
	TEST_VARIATION(2,		L"TO SQL_DATE")
	TEST_VARIATION(3,		L"To SQL_TIME")
	TEST_VARIATION(4,		L"To SQL_TIMESTAMP")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvertStamp)
//--------------------------------------------------------------------
// @class Test conversion from TIMESTAMP
//
class ConvertStamp : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvertStamp,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To SQL_CHAR
	int Variation_1();
	// @cmember To SQL_DATE
	int Variation_2();
	// @cmember To SQL_TIME
	int Variation_3();
	// @cmember To SQL_TIMESTAMP
	int Variation_4();
	// @cmember To SQL_VARCHAR
	int Variation_5();
	// @cmember To SQL_LONGVARCHAR
	int Variation_6();
	// @cmember To SQL_VARWCHAR
	int Variation_7();
	// @cmember To SQL_WCHAR
	int Variation_8();
	// @cmember To SQL_LONGVARWCHAR
	int Variation_9();
	// }}
};
// {{ TCW_TESTCASE(ConvertStamp)
#define THE_CLASS ConvertStamp
BEG_TEST_CASE(ConvertStamp, CScalar, L"Test conversion from TIMESTAMP")
	TEST_VARIATION(1,		L"To SQL_CHAR")
	TEST_VARIATION(2,		L"To SQL_DATE")
	TEST_VARIATION(3,		L"To SQL_TIME")
	TEST_VARIATION(4,		L"To SQL_TIMESTAMP")
	TEST_VARIATION(5,		L"To SQL_VARCHAR")
	TEST_VARIATION(6,		L"To SQL_LONGVARCHAR")
	TEST_VARIATION(7,		L"To SQL_VARWCHAR")
	TEST_VARIATION(8,		L"To SQL_WCHAR")
	TEST_VARIATION(9,		L"To SQL_LONGVARWCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromLongVarBin)
//--------------------------------------------------------------------
// @class Test conversions from SQL_LONGVARBINARY
//
class ConvFromLongVarBin : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromLongVarBin,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To Binary
	int Variation_1();
	// @cmember To Char
	int Variation_2();
	// @cmember To LongVarBinary
	int Variation_3();
	// @cmember To LongVarChar
	int Variation_4();
	// @cmember To varbinary
	int Variation_5();
	// @cmember To Varchar
	int Variation_6();
	// @cmember To SQL_WCHAR
	int Variation_7();
	// @cmember To SQL_VARWCHAR
	int Variation_8();
	// @cmember To SQL_LONGVARWCHAR
	int Variation_9();
	// }}
};
// {{ TCW_TESTCASE(ConvFromLongVarBin)
#define THE_CLASS ConvFromLongVarBin
BEG_TEST_CASE(ConvFromLongVarBin, CScalar, L"Test conversions from SQL_LONGVARBINARY")
	TEST_VARIATION(1,		L"To Binary")
	TEST_VARIATION(2,		L"To Char")
	TEST_VARIATION(3,		L"To LongVarBinary")
	TEST_VARIATION(4,		L"To LongVarChar")
	TEST_VARIATION(5,		L"To varbinary")
	TEST_VARIATION(6,		L"To Varchar")
	TEST_VARIATION(7,		L"To SQL_WCHAR")
	TEST_VARIATION(8,		L"To SQL_VARWCHAR")
	TEST_VARIATION(9,		L"To SQL_LONGVARWCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromLongVarChar)
//--------------------------------------------------------------------
// @class Test conversions from SQL_LONGVARCHAR
//
class ConvFromLongVarChar : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromLongVarChar,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To Bigint
	int Variation_1();
	// @cmember To Binary
	int Variation_2();
	// @cmember To Bit
	int Variation_3();
	// @cmember To Char
	int Variation_4();
	// @cmember To SQL_DATE
	int Variation_5();
	// @cmember To SQL_DECIMAL
	int Variation_6();
	// @cmember To SQL_DOUBLE
	int Variation_7();
	// @cmember To SQL_FLOAT
	int Variation_8();
	// @cmember To SQL_INTEGER
	int Variation_9();
	// @cmember To SQL_LONGVARBINARY
	int Variation_10();
	// @cmember To SQL_LONGVARCHAR
	int Variation_11();
	// @cmember To SQL_NUMERIC
	int Variation_12();
	// @cmember To SQL_REAL
	int Variation_13();
	// @cmember To SQL_SMALLINT
	int Variation_14();
	// @cmember To SQL_TIME
	int Variation_15();
	// @cmember To SQL_TIMESTAMP
	int Variation_16();
	// @cmember To TinyInt
	int Variation_17();
	// @cmember To VarBinary
	int Variation_18();
	// @cmember To Varchar
	int Variation_19();
	// @cmember To SQL_WCHAR
	int Variation_20();
	// @cmember To SQL_VARWCHAR
	int Variation_21();
	// @cmember To SQL_LONGVARWCHAR
	int Variation_22();
	// }}
};
// {{ TCW_TESTCASE(ConvFromLongVarChar)
#define THE_CLASS ConvFromLongVarChar
BEG_TEST_CASE(ConvFromLongVarChar, CScalar, L"Test conversions from SQL_LONGVARCHAR")
	TEST_VARIATION(1,		L"To Bigint")
	TEST_VARIATION(2,		L"To Binary")
	TEST_VARIATION(3,		L"To Bit")
	TEST_VARIATION(4,		L"To Char")
	TEST_VARIATION(5,		L"To SQL_DATE")
	TEST_VARIATION(6,		L"To SQL_DECIMAL")
	TEST_VARIATION(7,		L"To SQL_DOUBLE")
	TEST_VARIATION(8,		L"To SQL_FLOAT")
	TEST_VARIATION(9,		L"To SQL_INTEGER")
	TEST_VARIATION(10,		L"To SQL_LONGVARBINARY")
	TEST_VARIATION(11,		L"To SQL_LONGVARCHAR")
	TEST_VARIATION(12,		L"To SQL_NUMERIC")
	TEST_VARIATION(13,		L"To SQL_REAL")
	TEST_VARIATION(14,		L"To SQL_SMALLINT")
	TEST_VARIATION(15,		L"To SQL_TIME")
	TEST_VARIATION(16,		L"To SQL_TIMESTAMP")
	TEST_VARIATION(17,		L"To TinyInt")
	TEST_VARIATION(18,		L"To VarBinary")
	TEST_VARIATION(19,		L"To Varchar")
	TEST_VARIATION(20,		L"To SQL_WCHAR")
	TEST_VARIATION(21,		L"To SQL_VARWCHAR")
	TEST_VARIATION(22,		L"To SQL_LONGVARWCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromNumeric)
//--------------------------------------------------------------------
// @class Test conversions from SQL_NUMERIC
//
class ConvFromNumeric : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromNumeric,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ToBigInt
	int Variation_1();
	// @cmember To Binary
	int Variation_2();
	// @cmember To Bit
	int Variation_3();
	// @cmember To Char
	int Variation_4();
	// @cmember To SQL_DECIMAL
	int Variation_5();
	// @cmember To DOUBLE
	int Variation_6();
	// @cmember To FLOAT
	int Variation_7();
	// @cmember To INTEGER
	int Variation_8();
	// @cmember To LONGVARCHAR
	int Variation_9();
	// @cmember To NUMERIC
	int Variation_10();
	// @cmember To REAL
	int Variation_11();
	// @cmember To SMALLINT
	int Variation_12();
	// @cmember To TINYINT
	int Variation_13();
	// @cmember To VARCHAR
	int Variation_14();
	// @cmember TO WCHAR
	int Variation_15();
	// @cmember To WVARCHAR
	int Variation_16();
	// @cmember To WLONGVARCHAR
	int Variation_17();
	// }}
};
// {{ TCW_TESTCASE(ConvFromNumeric)
#define THE_CLASS ConvFromNumeric
BEG_TEST_CASE(ConvFromNumeric, CScalar, L"Test conversions from SQL_NUMERIC")
	TEST_VARIATION(1,		L"ToBigInt")
	TEST_VARIATION(2,		L"To Binary")
	TEST_VARIATION(3,		L"To Bit")
	TEST_VARIATION(4,		L"To Char")
	TEST_VARIATION(5,		L"To SQL_DECIMAL")
	TEST_VARIATION(6,		L"To DOUBLE")
	TEST_VARIATION(7,		L"To FLOAT")
	TEST_VARIATION(8,		L"To INTEGER")
	TEST_VARIATION(9,		L"To LONGVARCHAR")
	TEST_VARIATION(10,		L"To NUMERIC")
	TEST_VARIATION(11,		L"To REAL")
	TEST_VARIATION(12,		L"To SMALLINT")
	TEST_VARIATION(13,		L"To TINYINT")
	TEST_VARIATION(14,		L"To VARCHAR")
	TEST_VARIATION(15,		L"TO WCHAR")
	TEST_VARIATION(16,		L"To WVARCHAR")
	TEST_VARIATION(17,		L"To WLONGVARCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromReal)
//--------------------------------------------------------------------
// @class Test conversion from SQL_REAL
//
class ConvFromReal : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromReal,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To SQL_CHAR
	int Variation_1();
	// @cmember To DECIMAL
	int Variation_2();
	// @cmember To DOUBLE
	int Variation_3();
	// @cmember To FLOAT
	int Variation_4();
	// @cmember To INTEGER
	int Variation_5();
	// @cmember To NUMERIC
	int Variation_6();
	// @cmember To REAL
	int Variation_7();
	// @cmember To SMALLINT
	int Variation_8();
	// @cmember To TINYINT
	int Variation_9();
	// @cmember To VARCHAR
	int Variation_10();
	// @cmember To LONGVARCHAR
	int Variation_11();
	// @cmember To WCHAR
	int Variation_12();
	// @cmember To WVARCHAR
	int Variation_13();
	// @cmember To WLONGVARCHAR
	int Variation_14();
	// }}
};
// {{ TCW_TESTCASE(ConvFromReal)
#define THE_CLASS ConvFromReal
BEG_TEST_CASE(ConvFromReal, CScalar, L"Test conversion from SQL_REAL")
	TEST_VARIATION(1,		L"To SQL_CHAR")
	TEST_VARIATION(2,		L"To DECIMAL")
	TEST_VARIATION(3,		L"To DOUBLE")
	TEST_VARIATION(4,		L"To FLOAT")
	TEST_VARIATION(5,		L"To INTEGER")
	TEST_VARIATION(6,		L"To NUMERIC")
	TEST_VARIATION(7,		L"To REAL")
	TEST_VARIATION(8,		L"To SMALLINT")
	TEST_VARIATION(9,		L"To TINYINT")
	TEST_VARIATION(10,		L"To VARCHAR")
	TEST_VARIATION(11,		L"To LONGVARCHAR")
	TEST_VARIATION(12,		L"To WCHAR")
	TEST_VARIATION(13,		L"To WVARCHAR")
	TEST_VARIATION(14,		L"To WLONGVARCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromSmallInt)
//--------------------------------------------------------------------
// @class Test covnersion from SQL_SMALLINT
//
class ConvFromSmallInt : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromSmallInt,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To CHAR
	int Variation_1();
	// @cmember To DECIMAL
	int Variation_2();
	// @cmember To DOUBLE
	int Variation_3();
	// @cmember To FLOAT
	int Variation_4();
	// @cmember To INTEGER
	int Variation_5();
	// @cmember To NUMERIC
	int Variation_6();
	// @cmember To REAL
	int Variation_7();
	// @cmember To SMALLINT
	int Variation_8();
	// @cmember To TINYINT
	int Variation_9();
	// @cmember TO LONGVARCHAR
	int Variation_10();
	// @cmember To VARCHAR
	int Variation_11();
	// @cmember to WCHAR
	int Variation_12();
	// @cmember To WVARCHAR
	int Variation_13();
	// @cmember To WLONGVARCHAR
	int Variation_14();
	// }}
};
// {{ TCW_TESTCASE(ConvFromSmallInt)
#define THE_CLASS ConvFromSmallInt
BEG_TEST_CASE(ConvFromSmallInt, CScalar, L"Test covnersion from SQL_SMALLINT")
	TEST_VARIATION(1,		L"To CHAR")
	TEST_VARIATION(2,		L"To DECIMAL")
	TEST_VARIATION(3,		L"To DOUBLE")
	TEST_VARIATION(4,		L"To FLOAT")
	TEST_VARIATION(5,		L"To INTEGER")
	TEST_VARIATION(6,		L"To NUMERIC")
	TEST_VARIATION(7,		L"To REAL")
	TEST_VARIATION(8,		L"To SMALLINT")
	TEST_VARIATION(9,		L"To TINYINT")
	TEST_VARIATION(10,		L"TO LONGVARCHAR")
	TEST_VARIATION(11,		L"To VARCHAR")
	TEST_VARIATION(12,		L"to WCHAR")
	TEST_VARIATION(13,		L"To WVARCHAR")
	TEST_VARIATION(14,		L"To WLONGVARCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromTINYINT)
//--------------------------------------------------------------------
// @class Test conversions from SQL_TINYINT
//
class ConvFromTINYINT : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromTINYINT,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To SQL_CHAR
	int Variation_1();
	// @cmember To DECIMAL
	int Variation_2();
	// @cmember To Double
	int Variation_3();
	// @cmember To Float
	int Variation_4();
	// @cmember To Integer
	int Variation_5();
	// @cmember To Numeric
	int Variation_6();
	// @cmember To Real
	int Variation_7();
	// @cmember To LONGVARCHAR
	int Variation_8();
	// @cmember To Smallint
	int Variation_9();
	// @cmember To tinyint
	int Variation_10();
	// @cmember To VARCHAR
	int Variation_11();
	// @cmember To WCHAR
	int Variation_12();
	// @cmember To WVARCHAR
	int Variation_13();
	// @cmember To WLONGVARCHAR
	int Variation_14();
	// }}
};
// {{ TCW_TESTCASE(ConvFromTINYINT)
#define THE_CLASS ConvFromTINYINT
BEG_TEST_CASE(ConvFromTINYINT, CScalar, L"Test conversions from SQL_TINYINT")
	TEST_VARIATION(1,		L"To SQL_CHAR")
	TEST_VARIATION(2,		L"To DECIMAL")
	TEST_VARIATION(3,		L"To Double")
	TEST_VARIATION(4,		L"To Float")
	TEST_VARIATION(5,		L"To Integer")
	TEST_VARIATION(6,		L"To Numeric")
	TEST_VARIATION(7,		L"To Real")
	TEST_VARIATION(8,		L"To LONGVARCHAR")
	TEST_VARIATION(9,		L"To Smallint")
	TEST_VARIATION(10,		L"To tinyint")
	TEST_VARIATION(11,		L"To VARCHAR")
	TEST_VARIATION(12,		L"To WCHAR")
	TEST_VARIATION(13,		L"To WVARCHAR")
	TEST_VARIATION(14,		L"To WLONGVARCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromGUID)
//--------------------------------------------------------------------
// @class Test conversions from SQL_GUID
//
class ConvFromGUID : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromGUID,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To GUID
	int Variation_1();
	// @cmember To CHAR
	int Variation_2();
	// @cmember To VARCHAR
	int Variation_3();
	// @cmember To LONGVARCHAR
	int Variation_4();
	// @cmember To WCHAR
	int Variation_5();
	// @cmember To WVARCHAR
	int Variation_6();
	// @cmember To WLONGVARCHAR
	int Variation_7();
	// }}
};
// {{ TCW_TESTCASE(ConvFromGUID)
#define THE_CLASS ConvFromGUID
BEG_TEST_CASE(ConvFromGUID, CScalar, L"Test conversions from SQL_GUID")
	TEST_VARIATION(1,		L"To GUID")
	TEST_VARIATION(2,		L"To CHAR")
	TEST_VARIATION(3,		L"To VARCHAR")
	TEST_VARIATION(4,		L"To LONGVARCHAR")
	TEST_VARIATION(5,		L"To WCHAR")
	TEST_VARIATION(6,		L"To WVARCHAR")
	TEST_VARIATION(7,		L"To WLONGVARCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromWCHAR)
//--------------------------------------------------------------------
// @class Test conversion from SQL_WCHAR
//
class ConvFromWCHAR : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromWCHAR,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To BigInt
	int Variation_1();
	// @cmember To Binary
	int Variation_2();
	// @cmember To Bit
	int Variation_3();
	// @cmember To Char
	int Variation_4();
	// @cmember To SQL_DATE
	int Variation_5();
	// @cmember To Decimal
	int Variation_6();
	// @cmember To Double
	int Variation_7();
	// @cmember To Float
	int Variation_8();
	// @cmember To Integer
	int Variation_9();
	// @cmember To LongVarBinary
	int Variation_10();
	// @cmember To LongVarChar
	int Variation_11();
	// @cmember To Numeric
	int Variation_12();
	// @cmember To Real
	int Variation_13();
	// @cmember To Smallint
	int Variation_14();
	// @cmember To Time
	int Variation_15();
	// @cmember To TimeStamp
	int Variation_16();
	// @cmember To TinyInt
	int Variation_17();
	// @cmember To VarBinary
	int Variation_18();
	// @cmember To VARCHAR
	int Variation_19();
	// @cmember To WCHAR
	int Variation_20();
	// @cmember To WVARCHAR
	int Variation_21();
	// @cmember To WLONGVARCHAR
	int Variation_22();
	// @cmember To SQL_GUID
	int Variation_23();
	// }}
};
// {{ TCW_TESTCASE(ConvFromWCHAR)
#define THE_CLASS ConvFromWCHAR
BEG_TEST_CASE(ConvFromWCHAR, CScalar, L"Test conversion from SQL_WCHAR")
	TEST_VARIATION(1,		L"To BigInt")
	TEST_VARIATION(2,		L"To Binary")
	TEST_VARIATION(3,		L"To Bit")
	TEST_VARIATION(4,		L"To Char")
	TEST_VARIATION(5,		L"To SQL_DATE")
	TEST_VARIATION(6,		L"To Decimal")
	TEST_VARIATION(7,		L"To Double")
	TEST_VARIATION(8,		L"To Float")
	TEST_VARIATION(9,		L"To Integer")
	TEST_VARIATION(10,		L"To LongVarBinary")
	TEST_VARIATION(11,		L"To LongVarChar")
	TEST_VARIATION(12,		L"To Numeric")
	TEST_VARIATION(13,		L"To Real")
	TEST_VARIATION(14,		L"To Smallint")
	TEST_VARIATION(15,		L"To Time")
	TEST_VARIATION(16,		L"To TimeStamp")
	TEST_VARIATION(17,		L"To TinyInt")
	TEST_VARIATION(18,		L"To VarBinary")
	TEST_VARIATION(19,		L"To VARCHAR")
	TEST_VARIATION(20,		L"To WCHAR")
	TEST_VARIATION(21,		L"To WVARCHAR")
	TEST_VARIATION(22,		L"To WLONGVARCHAR")
	TEST_VARIATION(23,		L"To SQL_GUID")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromVarBin)
//--------------------------------------------------------------------
// @class Test conversion from SQL_VARBINARY
//
class ConvFromVarBin : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromVarBin,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To VARBINARY
	int Variation_1();
	// @cmember to CHAR
	int Variation_2();
	// @cmember to WCHAR
	int Variation_3();
	// }}
};
// {{ TCW_TESTCASE(ConvFromVarBin)
#define THE_CLASS ConvFromVarBin
BEG_TEST_CASE(ConvFromVarBin, CScalar, L"Test conversion from SQL_VARBINARY")
	TEST_VARIATION(1,		L"To VARBINARY")
	TEST_VARIATION(2,		L"to CHAR")
	TEST_VARIATION(3,		L"to WCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromVarChar)
//--------------------------------------------------------------------
// @class Conversion from SQL_VARCHAR
//
class ConvFromVarChar : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromVarChar,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To NUMERIC
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(ConvFromVarChar)
#define THE_CLASS ConvFromVarChar
BEG_TEST_CASE(ConvFromVarChar, CScalar, L"Conversion from SQL_VARCHAR")
	TEST_VARIATION(1,		L"To NUMERIC")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ConvFromDecimal)
//--------------------------------------------------------------------
// @class Test conversion from SQL_DECIMAL
//
class ConvFromDecimal : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ConvFromDecimal,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember To CHAR
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(ConvFromDecimal)
#define THE_CLASS ConvFromDecimal
BEG_TEST_CASE(ConvFromDecimal, CScalar, L"Test conversion from SQL_DECIMAL")
	TEST_VARIATION(1,		L"To CHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Y2K_Tests)
//--------------------------------------------------------------------
// @class Test timedate scalar funcs relating to Y2k
//
class Y2K_Tests : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Y2K_Tests,CScalar);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember DAYNAME(timestamp
	int Variation_1();
	// @cmember DAYNAME(date
	int Variation_2();
	// @cmember DAYOFMONTH(timestamp
	int Variation_3();
	// @cmember DAYOFMONTH(date
	int Variation_4();
	// @cmember DAYOFWEEK(timestamp
	int Variation_5();
	// @cmember DAYOFWEEK(date
	int Variation_6();
	// @cmember DAYOFYEAR(timestamp
	int Variation_7();
	// @cmember DAYOFYEAR(date
	int Variation_8();
	// @cmember MONTH(timestamp
	int Variation_9();
	// @cmember MONTH(date
	int Variation_10();
	// @cmember QUARTER(timestamp
	int Variation_11();
	// @cmember QUARTER(date
	int Variation_12();
	// @cmember TIMESTAMPADD(timestamps
	int Variation_13();
	// @cmember TIMESTAMPADD(dates
	int Variation_14();
	// @cmember TIMESTAMPDIFF(timestamps
	int Variation_15();
	// @cmember TIMESTAMPDIFF(dates
	int Variation_16();
	// @cmember WEEK(timestamp
	int Variation_17();
	// @cmember WEEK(dates
	int Variation_18();
	// @cmember YEAR(timestamps
	int Variation_19();
	// @cmember YEAR(dates
	int Variation_20();
	// @cmember HOUR(timestamps
	int Variation_21();
	// @cmember HOUR(date
	int Variation_22();
	// @cmember DAYOFYEAR(timestamp
	int Variation_23();
	// @cmember DAYOFWEEK(timestamp
	int Variation_24();
	// @cmember DAYOFWEEK(timestamp
	int Variation_25();
	// @cmember WEEK(dates
	int Variation_26();
	// @cmember DAYOFYEAR(timestamp
	int Variation_27();
	// @cmember DAYOFYEAR(timestamp
	int Variation_28();
	// @cmember DAYOFYEAR(timestamp
	int Variation_29();
	// @cmember DAYOFWEEK(timestamp
	int Variation_30();
	// @cmember QUARTER(timestamp
	int Variation_31();
	// @cmember QUARTER(timestamp
	int Variation_32();
	// @cmember DAYOFYEAR(timestamp
	int Variation_33();
	// @cmember DAYOFYEAR(timestamp
	int Variation_34();
	// @cmember DAYOFWEEK(timestamp
	int Variation_35();
	// @cmember DAYOFWEEK(timestamp
	int Variation_36();
	// @cmember DAYOFWEEK(timestamp
	int Variation_37();
	// @cmember DAYOFYEAR(timestamp
	int Variation_38();
	// @cmember DAYOFWEEK(timestamp
	int Variation_39();
	// @cmember DAYOFWEEK(timestamp
	int Variation_40();
	// @cmember Nested TIMESTAMPADD timestamp
	int Variation_41();
	// @cmember Nested TIMESTAMPADD date
	int Variation_42();
	// @cmember Time canonical literal
	int Variation_43();
	// }}
} ;
// {{ TCW_TESTCASE(Y2K_Tests)
#define THE_CLASS Y2K_Tests
BEG_TEST_CASE(Y2K_Tests, CScalar, L"Test timedate scalar funcs relating to Y2k")
	TEST_VARIATION(1,		L"DAYNAME(timestamp")
	TEST_VARIATION(2,		L"DAYNAME(date")
	TEST_VARIATION(3,		L"DAYOFMONTH(timestamp")
	TEST_VARIATION(4,		L"DAYOFMONTH(date")
	TEST_VARIATION(5,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(6,		L"DAYOFWEEK(date")
	TEST_VARIATION(7,		L"DAYOFYEAR(timestamp")
	TEST_VARIATION(8,		L"DAYOFYEAR(date")
	TEST_VARIATION(9,		L"MONTH(timestamp")
	TEST_VARIATION(10,		L"MONTH(date")
	TEST_VARIATION(11,		L"QUARTER(timestamp")
	TEST_VARIATION(12,		L"QUARTER(date")
	TEST_VARIATION(13,		L"TIMESTAMPADD(timestamps")
	TEST_VARIATION(14,		L"TIMESTAMPADD(dates")
	TEST_VARIATION(15,		L"TIMESTAMPDIFF(timestamps")
	TEST_VARIATION(16,		L"TIMESTAMPDIFF(dates")
	TEST_VARIATION(17,		L"WEEK(timestamp")
	TEST_VARIATION(18,		L"WEEK(dates")
	TEST_VARIATION(19,		L"YEAR(timestamps")
	TEST_VARIATION(20,		L"YEAR(dates")
	TEST_VARIATION(21,		L"HOUR(timestamps")
	TEST_VARIATION(22,		L"HOUR(date")
	TEST_VARIATION(23,		L"DAYOFYEAR(timestamp")
	TEST_VARIATION(24,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(25,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(26,		L"WEEK(dates")
	TEST_VARIATION(27,		L"DAYOFYEAR(timestamp")
	TEST_VARIATION(28,		L"DAYOFYEAR(timestamp")
	TEST_VARIATION(29,		L"DAYOFYEAR(timestamp")
	TEST_VARIATION(30,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(31,		L"QUARTER(timestamp")
	TEST_VARIATION(32,		L"QUARTER(timestamp")
	TEST_VARIATION(33,		L"DAYOFYEAR(timestamp")
	TEST_VARIATION(34,		L"DAYOFYEAR(timestamp")
	TEST_VARIATION(35,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(36,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(37,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(38,		L"DAYOFYEAR(timestamp")
	TEST_VARIATION(39,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(40,		L"DAYOFWEEK(timestamp")
	TEST_VARIATION(41,		L"Nested TIMESTAMPADD timestamp")
	TEST_VARIATION(42,		L"Nested TIMESTAMPADD date")
	TEST_VARIATION(43,		L"Time canonical literal")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(EURO)
//--------------------------------------------------------------------
// @class Ansi Unicode conversion cases involving EURO symbol
//
class EURO : public CScalar {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	WCHAR	m_wszEuro[2];
	BOOL	m_fPass;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(EURO,CScalar);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Convert SQL_CHAR to SQL_CHAR
	int Variation_1();
	// @cmember Convert SQL_VARCHAR to SQL_VARCHAR
	int Variation_2();
	// @cmember Convert SQL_LONGVARCHAR to SQL_LONGVARCHAR
	int Variation_3();
	// @cmember Convert SQL_WCHAR to SQL_WCHAR
	int Variation_4();
	// @cmember Convert SQL_WVARCHAR to SQL_WVARCHAR
	int Variation_5();
	// @cmember Convert SQL_WLONGVARCHAR to SQL_WLONGVARCHAR
	int Variation_6();
	// @cmember Convert SQL_CHAR to SQL_WCHAR
	int Variation_7();
	// @cmember Convert SQL_CHAR to SQL_WVARCHAR
	int Variation_8();
	// @cmember Convert SQL_CHAR to SQL_WLONGVARCHAR
	int Variation_9();
	// @cmember Convert SQL_VARCHAR to SQLWCHAR
	int Variation_10();
	// @cmember Convert SQL_VARCHAR to SQL_WVARCHAR
	int Variation_11();
	// @cmember Convert SQL_VARCHAR to SQL_WLONGVARCHAR
	int Variation_12();
	// @cmember Convert SQL_LONGVARCHAR to SQL_WCHAR
	int Variation_13();
	// @cmember Convert SQL_LONGVARCHAR to SQL_WVARCHAR
	int Variation_14();
	// @cmember Convert SQL_LONGVARCHAR to SQL_WLONGVARCHAR
	int Variation_15();
	// @cmember Convert SQL_WCHAR to SQL_CHAR
	int Variation_16();
	// @cmember Convert SQL_WCHAR to SQL_VARCHAR
	int Variation_17();
	// @cmember Convert SQL_WCHAR to SQL_LONGVARCHAR
	int Variation_18();
	// @cmember Convert SQL_WVARCHAR to SQL_CHAR
	int Variation_19();
	// @cmember Convert SQL_WVARCHAR to SQL_VARCHAR
	int Variation_20();
	// @cmember Convert SQL_WVARCHAR to SQL_LONGVARCHAR
	int Variation_21();
	// @cmember Convert SQL_WLONGVARCHAR to SQL_CHAR
	int Variation_22();
	// @cmember Convert SQL_WLONGVARCHAR to SQL_VARCHAR
	int Variation_23();
	// @cmember Convert SQL_WLONGVARCHAR to SQL_LONGVARCHAR
	int Variation_24();
	// }}
};
// {{ TCW_TESTCASE(EURO)
#define THE_CLASS EURO
BEG_TEST_CASE(EURO, CScalar, L"Ansi Unicode conversion cases involving EURO symbol")
	TEST_VARIATION(1,		L"Convert SQL_CHAR to SQL_CHAR")
	TEST_VARIATION(2,		L"Convert SQL_VARCHAR to SQL_VARCHAR")
	TEST_VARIATION(3,		L"Convert SQL_LONGVARCHAR to SQL_LONGVARCHAR")
	TEST_VARIATION(4,		L"Convert SQL_WCHAR to SQL_WCHAR")
	TEST_VARIATION(5,		L"Convert SQL_WVARCHAR to SQL_WVARCHAR")
	TEST_VARIATION(6,		L"Convert SQL_WLONGVARCHAR to SQL_WLONGVARCHAR")
	TEST_VARIATION(7,		L"Convert SQL_CHAR to SQL_WCHAR")
	TEST_VARIATION(8,		L"Convert SQL_CHAR to SQL_WVARCHAR")
	TEST_VARIATION(9,		L"Convert SQL_CHAR to SQL_WLONGVARCHAR")
	TEST_VARIATION(10,		L"Convert SQL_VARCHAR to SQLWCHAR")
	TEST_VARIATION(11,		L"Convert SQL_VARCHAR to SQL_WVARCHAR")
	TEST_VARIATION(12,		L"Convert SQL_VARCHAR to SQL_WLONGVARCHAR")
	TEST_VARIATION(13,		L"Convert SQL_LONGVARCHAR to SQL_WCHAR")
	TEST_VARIATION(14,		L"Convert SQL_LONGVARCHAR to SQL_WVARCHAR")
	TEST_VARIATION(15,		L"Convert SQL_LONGVARCHAR to SQL_WLONGVARCHAR")
	TEST_VARIATION(16,		L"Convert SQL_WCHAR to SQL_CHAR")
	TEST_VARIATION(17,		L"Convert SQL_WCHAR to SQL_VARCHAR")
	TEST_VARIATION(18,		L"Convert SQL_WCHAR to SQL_LONGVARCHAR")
	TEST_VARIATION(19,		L"Convert SQL_WVARCHAR to SQL_CHAR")
	TEST_VARIATION(20,		L"Convert SQL_WVARCHAR to SQL_VARCHAR")
	TEST_VARIATION(21,		L"Convert SQL_WVARCHAR to SQL_LONGVARCHAR")
	TEST_VARIATION(22,		L"Convert SQL_WLONGVARCHAR to SQL_CHAR")
	TEST_VARIATION(23,		L"Convert SQL_WLONGVARCHAR to SQL_VARCHAR")
	TEST_VARIATION(24,		L"Convert SQL_WLONGVARCHAR to SQL_LONGVARCHAR")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(23, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, StringFunctions)
	TEST_CASE(2, NumFunctions)
	TEST_CASE(3, DateTimeFn)
	TEST_CASE(4, SystemFunctions)
	TEST_CASE(5, OuterJoins)
	TEST_CASE(6, LikeEscape)
	TEST_CASE(7, LiteralDateTimeClauses)
	TEST_CASE(8, ConvertClause)
	TEST_CASE(9, ConvertFromTim)
	TEST_CASE(10, ConvertStamp)
	TEST_CASE(11, ConvFromLongVarBin)
	TEST_CASE(12, ConvFromLongVarChar)
	TEST_CASE(13, ConvFromNumeric)
	TEST_CASE(14, ConvFromReal)
	TEST_CASE(15, ConvFromSmallInt)
	TEST_CASE(16, ConvFromTINYINT)
	TEST_CASE(17, ConvFromGUID)
	TEST_CASE(18, ConvFromWCHAR)
	TEST_CASE(19, ConvFromVarBin)
	TEST_CASE(20, ConvFromVarChar)
	TEST_CASE(21, ConvFromDecimal)
	TEST_CASE(22, Y2K_Tests)
	TEST_CASE(23, EURO)
END_TEST_MODULE()
// }}


// {{ TCW_TC_PROTOTYPE(StringFunctions)
//*-----------------------------------------------------------------------
//|	Test Case:		StringFunctions - Test scalar string functions
//|	Created:			05/03/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL StringFunctions::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		// TO DO:  Add your own code here
		
		// NEED To see if the provider supports string scalar functions
		// There is no way to do this in OLE-DB, so we may have to hard-code.
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ASCII Bz
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_1()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnAscii));

	if(IsSupportedScalar(SQL_FN_STR_ASCII))
		return CheckQry(L"66", CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc CHAR 102
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_2()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnChar));

	if(IsSupportedScalar(SQL_FN_STR_CHAR))		
		return CheckQry(L"f", CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc CONCAT Fire Truck
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_3()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnConcat));

	if(IsSupportedScalar(SQL_FN_STR_CONCAT))
		return CheckQry(L"FireTruck",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LTRIM  a BCDef
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_4()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnInsert));
	
	if(IsSupportedScalar(SQL_FN_STR_INSERT))
		return CheckQry(L"Firehosek",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc LENGTH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_5()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnLeft));

	if(IsSupportedScalar(SQL_FN_STR_LEFT))
		return CheckQry(L"aB",CHECKSTR);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc LOCATE st in forest estates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_6()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnLocate));

	if(IsSupportedScalar(SQL_FN_STR_LOCATE))
		return CheckQry(L"9",CHECKINT);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc LCASE fIrEtRucK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_7()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnLcase));

	if(IsSupportedScalar(SQL_FN_STR_LCASE))
		return CheckQry(L"firetruck",CHECKSTR);			
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc REPEAT tq 3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_8()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnRepeat));

	if(IsSupportedScalar(SQL_FN_STR_REPEAT))
		return CheckQry(L"t qt qt q",CHECKSTR);	
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc REPLACE forest estates  es truck
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_9()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnReplace));

	if(IsSupportedScalar(SQL_FN_STR_REPLACE))
		return CheckQry(L"fortruckt trucktattruck",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc RIGHT forest estates 7
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_10()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnRight));

	if(IsSupportedScalar(SQL_FN_STR_RIGHT))
		return CheckQry(L"estates",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc RTRIM  forest estates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_11()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnRtrim));

	if(IsSupportedScalar(SQL_FN_STR_RTRIM))
		return CheckQry(L"  forest estates",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc SUBSTRING  forest estates 5 4
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_12()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnSubstring));
	
	if(IsSupportedScalar(SQL_FN_STR_SUBSTRING))
		return CheckQry(L"rest",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc UCASE forest estates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_13()
{
	swprintf(m_wszQuery, g_wszScalarFmt,GetidsString(m_buf,idsFnUcase));
	
	if(IsSupportedScalar(SQL_FN_STR_UCASE))
		return CheckQry(L"  FOREST ESTATES  ",CHECKSTR);			
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DIFFERENCE c ab
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_14()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDiff));

	if(IsSupportedScalar(SQL_FN_STR_DIFFERENCE))
		return CheckQry(L"2",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc SOUNDEX ab
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_15()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnSoundex));

	if(IsSupportedScalar(SQL_FN_STR_SOUNDEX))
		return CheckQry(L"A100",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc SPACE 5
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_16()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnSpace));

	if(IsSupportedScalar(SQL_FN_STR_SPACE))
		return CheckQry(L"     ",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc CONCAT nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_17()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnConcatNested));

	if(IsSupportedScalar(SQL_FN_STR_CONCAT))
		return CheckQry(L"AAABBB",CHECKSTR);			
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc LEFT nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_18()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnLeftNested));

	if(IsSupportedScalar(SQL_FN_STR_LEFT))
		return CheckQry(L"ab",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc RIGHT nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_19()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnRightNested));
	
	if(IsSupportedScalar(SQL_FN_STR_RIGHT))
		return CheckQry(L"fg",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc LOCATE nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_20()
{
	//	check: SQL Server supports LOCATE2
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnLocateNested));

	if(IsSupportedScalar(SQL_FN_STR_LOCATE_2))
		return CheckQry(L"3",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc SUBSTRING nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_21()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnSubstringNested));

	if(IsSupportedScalar(SQL_FN_STR_SUBSTRING))
		return CheckQry(L"bc",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DIFFERENCE nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_22()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDiffNested));

	if(IsSupportedScalar(SQL_FN_STR_DIFFERENCE))
		return CheckQry(L"2",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc REPEAT nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_23()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnRepeatNested));

	if(IsSupportedScalar(SQL_FN_STR_REPEAT))
		return CheckQry(L"abcabcabc",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc REPLACE nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_24()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnReplaceNested));
	
	if(IsSupportedScalar(SQL_FN_STR_REPLACE))
		return CheckQry(L"aXbXdX",CHECKSTR);			
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc INSERT nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_25()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnInsertNested));
	
	if(IsSupportedScalar(SQL_FN_STR_INSERT))
		return CheckQry(L"aXYefg",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc mulit-INSERT nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_26()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnMultiInsert));
	
	if(IsSupportedScalar(SQL_FN_STR_INSERT))
		return CheckQry(L"123XWZ5678",CHECKSTR);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Multi CONCAT nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_27()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnMultiConcat));

	if((IsSupportedScalar(SQL_FN_STR_CONCAT)) && (IsSupportedScalar(SQL_FN_STR_UCASE)) && (IsSupportedScalar(SQL_FN_STR_SPACE)))
		return CheckQry(L"C    ",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc STRING and NUMERIC
//
// @rdesc TEST_PASS or TEST_FAIL
//
int StringFunctions::Variation_28()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnStringAndNumeric));

	if(IsSupportedScalar(SQL_FN_STR_CHAR))
		return CheckQry(L"c",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL StringFunctions::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(NumFunctions)
//*-----------------------------------------------------------------------
//|	Test Case:		NumFunctions - Test numeric scalar functions
//|	Created:			05/05/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL NumFunctions::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ABS -123
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_1()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnAbs));
	
	if(IsSupportedNumeric(SQL_FN_NUM_ABS))
		return CheckQry(L"123",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ACOS 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_2()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnAcos));

	if(IsSupportedNumeric(SQL_FN_NUM_ACOS))
		return CheckQry(L"0",CHECKFLOAT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc ASIN -1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_3()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnAsin));
			
	if(IsSupportedNumeric(SQL_FN_NUM_ASIN))											
		return CheckQry(L"-1.5707963267949",CHECKFLOAT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ATAN 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_4()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnAtan));	
	
	if(IsSupportedNumeric(SQL_FN_NUM_ATAN))		
		return CheckQry(L"1.10714871779409",CHECKFLOAT);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ATAN2  2  1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_5()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnAtan2));
	
	if(IsSupportedNumeric(SQL_FN_NUM_ATAN2))		
		return CheckQry(L"1.10714871779409",CHECKFLOAT);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc CEILING 1.09
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_6()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnCeiling));

	if(IsSupportedNumeric(SQL_FN_NUM_CEILING))
		return CheckQry(L"2",CHECKINT);				
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc COS  0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_7()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnCos));
		
	if(IsSupportedNumeric(SQL_FN_NUM_COS))				
		return CheckQry(L"1",CHECKFLOAT);				
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc COT  0.5
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_8()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnCot));
	
	if(IsSupportedNumeric(SQL_FN_NUM_COT))
		return CheckQry(L"1.83048772171245",CHECKFLOAT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc EXP   1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_9()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnExp));
	
	if(IsSupportedNumeric(SQL_FN_NUM_EXP))						
		return CheckQry(L"2.71828182845905",CHECKFLOAT);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc FLOOR  1.09
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_10()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnFloor));
				
	if(IsSupportedNumeric(SQL_FN_NUM_FLOOR))
		return CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc LOG   10
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_11()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnLog));

	if(IsSupportedNumeric(SQL_FN_NUM_LOG))
		return CheckQry(L"2.30258509299405",CHECKFLOAT);			
	else
		return TEST_SKIPPED;
}		
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc MOD  10  6
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_12()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnMod));
		
	if(IsSupportedNumeric(SQL_FN_NUM_MOD))			
		return CheckQry(L"4",CHECKFLOAT);			
	else
		return TEST_SKIPPED;

}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc PI
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_13()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnPi));
		
	if(IsSupportedNumeric(SQL_FN_NUM_PI))			
		return CheckQry(L"3.14159265358979",CHECKFLOAT);			
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc RAND  10
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_14()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnRand));

	if(IsSupportedNumeric(SQL_FN_NUM_RAND))			
		return CheckQry(L"IGNORE",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc SIGN   10
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_15()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnSign));
	
	if(IsSupportedNumeric(SQL_FN_NUM_SIGN))
		return CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc SIN   .05
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_16()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnSin));
	
	if(IsSupportedNumeric(SQL_FN_NUM_SIN))			
		return CheckQry(L"0.0499791692706783",CHECKFLOAT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc SQRT   9
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_17()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnSqrt));
		
	if(IsSupportedNumeric(SQL_FN_NUM_SQRT))
		return CheckQry(L"3",CHECKFLOAT);	
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc TAN   2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_18()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnTan));
		
	if(IsSupportedNumeric(SQL_FN_NUM_TAN))			
		return CheckQry(L"-2.18503986326152",CHECKFLOAT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc DEGREES  3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_19()
{
 	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDegrees));
		
	if(IsSupportedNumeric(SQL_FN_NUM_DEGREES))
		return CheckQry(L"171",CHECKFLOAT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc LOG10    97.1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_20()
{
 	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnLog10));
	
	if(IsSupportedNumeric(SQL_FN_NUM_LOG10))
		return CheckQry(L"1.987219229908",CHECKFLOAT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc POWER  9.6   3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_21()
{
  	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnPower));
	
	// POWER loses some precision - so use CHECKREAL
	if(IsSupportedNumeric(SQL_FN_NUM_POWER))		
		return CheckQry(L"884.736",CHECKREAL);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc RADIANS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_22()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnRadians));

	if(IsSupportedNumeric(SQL_FN_NUM_RADIANS))
		return CheckQry(L".148352986419518020",CHECKFLOAT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROUND  97.56   1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_23()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnRound));
		
	if(IsSupportedNumeric(SQL_FN_NUM_ROUND))
		return CheckQry(L"97.6",CHECKFLOAT);		
	else
		return TEST_SKIPPED;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc TRUNCATE  97.56   1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NumFunctions::Variation_24()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnTruncate));
		
	if(IsSupportedNumeric(SQL_FN_NUM_TRUNCATE))
		return CheckQry(L"97.5",CHECKFLOAT);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL NumFunctions::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DateTimeFn)
//*-----------------------------------------------------------------------
//|	Test Case:		DateTimeFn - Test date time scalar functions
//|	Created:			05/05/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DateTimeFn::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc NOW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_1()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnNow));

	if(IsSupportedDateTime(SQL_FN_TD_NOW))
		return CheckQry(L"IGNORE",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc CURDATE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_2()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnCurdate));

	if(IsSupportedDateTime(SQL_FN_TD_CURDATE))
		return CheckQry(L"IGNORE",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DAYOFMONTH  1991 02 26
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_3()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDayofmonth));
		
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFMONTH))				
		return CheckQry(L"26",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DATOFWEEK  1991 02 26
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_4()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDayofweek));

	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))
		return CheckQry(L"3", CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR 1991 02 26
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_5()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDayofyear));
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"57",CHECKINT);			
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc MONTH  1991 02 26
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_6()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnMonth));
	
	if(IsSupportedDateTime(SQL_FN_TD_MONTH))			
		return CheckQry(L"2",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc QUARTER 1991 02 26
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_7()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnQuarter));
	
	if(IsSupportedDateTime(SQL_FN_TD_QUARTER))			
		return 	CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc WEEK  1991 02 26
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_8()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnWeek));
	
	if(IsSupportedDateTime(SQL_FN_TD_WEEK))			
		return CheckQry(L"9",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc YEAR 1991 02 26
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_9()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnYear));

	if(IsSupportedDateTime(SQL_FN_TD_YEAR))				
		return CheckQry(L"1991",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc CURTIME
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_10()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnCurtime));
		
	if(IsSupportedDateTime(SQL_FN_TD_CURTIME))
		return CheckQry(L"IGNORE",CHECKSTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc HOUR 14:49:19
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_11()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnHour));
	
	if(IsSupportedDateTime(SQL_FN_TD_HOUR))				
		return CheckQry(L"14",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc MINUTE  14:49:19
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_12()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnMinute));
		
	if(IsSupportedDateTime(SQL_FN_TD_MINUTE))
		return CheckQry(L"49",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc SECOND  14:49:19
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_13()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnSecond));
	
	if(IsSupportedDateTime(SQL_FN_TD_SECOND))		
		return CheckQry(L"19",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DAYNAME d 1994 12 12
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_14()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDayname));

	if(IsSupportedDateTime(SQL_FN_TD_DAYNAME))
		return CheckQry(L"Monday",CHECKISTR);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc MONTHNAME d 1994 12 12
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_15()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnMonthname));
	
	if(IsSupportedDateTime(SQL_FN_TD_MONTHNAME))
		return CheckQry(L"December",CHECKISTR);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc TIMESTAMPADD SQL_TSI_DAY  d 1994 12 12
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_16()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnTimestampadd));

	if(IsSupportedDateTime(SQL_FN_TD_TIMESTAMPADD))
		return CheckQry(L"1994-12-14 00:00:00.000",CHECKTIMESTAMP);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc TIMESTAMPDIFF SQL_TSI_MONTH 11 15 94  12 12 94
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_17()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnTimestampdiff));
		
	if(IsSupportedDateTime(SQL_FN_TD_TIMESTAMPDIFF))			
		return CheckQry(L"1",CHECKINT);			
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc CURRENT_TIME
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_18()
{
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn CURRENT_TIME");
		
	if(IsSupportedDateTime(SQL_FN_TD_CURRENT_TIME ))			
		return CheckQry(L"IGNORE",CHECKSTR);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc CURRENT_TIME()
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_19()
{
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn CURRENT_TIME()");
		
	if(IsSupportedDateTime(SQL_FN_TD_CURRENT_TIME ))			
		return CheckQry(L"IGNORE",CHECKSTR);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc CURRENT_TIMESTAMP
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_20()
{
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn CURRENT_TIMESTAMP");
		
	if(IsSupportedDateTime(SQL_FN_TD_CURRENT_TIME ))			
		return CheckQry(L"IGNORE",CHECKSTR);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc CURRENT_TIMESTAMP
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DateTimeFn::Variation_21()
{
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn CURRENT_TIMESTAMP()");
		
	if(IsSupportedDateTime(SQL_FN_TD_CURRENT_TIME ))			
		return CheckQry(L"IGNORE",CHECKSTR);		
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DateTimeFn::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(SystemFunctions)
//*-----------------------------------------------------------------------
//|	Test Case:		SystemFunctions - Test system scalar functions
//|	Created:			05/05/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SystemFunctions::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc USER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SystemFunctions::Variation_1()
{
	WCHAR	*wszUserName = NULL;
	BOOL	fPass = TEST_FAIL;	

	if (!SupportedProperty(DBPROP_USERNAME, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize,DATASOURCE_INTERFACE))
		return TEST_SKIPPED;

	if (!COMPARE(GetProperty(DBPROP_USERNAME, DBPROPSET_DATASOURCEINFO,	m_pIDBInitialize, &wszUserName),TRUE))
		return TEST_FAIL;

	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnUser));

	if(IsSupportedSystem(SQL_FN_SYS_USERNAME))
		fPass = CheckQry(wszUserName,CHECKSTR);
	else
		fPass = TEST_SKIPPED;

	PROVIDER_FREE(wszUserName);
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DATABASE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SystemFunctions::Variation_2()
{
	WCHAR	*wszDatabase = NULL;
	BOOL	fPass = TEST_FAIL;	

	if(!IsSupportedSystem(SQL_FN_SYS_DBNAME))
		return TEST_SKIPPED;

	if (!SupportedProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, m_pIDBInitialize,DATASOURCE_INTERFACE))
		return TEST_FAIL;

	if (!COMPARE(GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, m_pIDBInitialize, &wszDatabase),TRUE))
		return TEST_FAIL;

	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDatabase));
	
	fPass = CheckQry(wszDatabase,CHECKSTR);
	

	PROVIDER_FREE(wszDatabase);
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IFNULL NULL  4
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SystemFunctions::Variation_3()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnIfnull1));
		
	if(IsSupportedSystem(SQL_FN_SYS_IFNULL))
		return CheckQry(L"4",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IFNULL   -1  4
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SystemFunctions::Variation_4()
{
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnIfnull2));

	if(IsSupportedSystem(SQL_FN_SYS_IFNULL))
		return CheckQry(L"-1",CHECKINT);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL SystemFunctions::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(OuterJoins)
//*-----------------------------------------------------------------------
//|	Test Case:		OuterJoins - Test outer join canonical
//|	Created:			05/04/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL OuterJoins::Init()
{
	CCol	TempCol;

	m_pTable2 = NULL;
	m_pTable3 = NULL;
	m_wszGlobalTableColName = NULL;
	m_wszTable2ColName = NULL;
	m_wszTable3ColName = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		m_pTable2 = new CTable(m_pThisTestModule->m_pIUnknown2, 0);
		m_pTable3 = new CTable(m_pThisTestModule->m_pIUnknown2, 0);
		if (!m_pTable2 || !m_pTable3)
			return FALSE;

		if(	!SUCCEEDED(m_pTable2->CreateTable(10)) ||
			!SUCCEEDED(m_pTable3->CreateTable(10)) )
			return FALSE;

		if(FAILED(m_pTable2->GetFirstNumericCol(&TempCol)))
			return TEST_SKIPPED;

		m_ulNumCol = TempCol.GetColNum();

		m_wszTable2ColName = wcsDuplicate(TempCol.GetColName());
		
		g_pTable->GetColInfo(m_ulNumCol, TempCol);
		m_wszGlobalTableColName = wcsDuplicate(TempCol.GetColName());

		m_pTable3->GetColInfo(m_ulNumCol, TempCol);
		m_wszTable3ColName = wcsDuplicate(TempCol.GetColName());

		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Left OJ
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_1()
{
	swprintf(m_wszQuery, 			
			GetidsString(m_buf,idsShortLJoin),
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName);
	
	return ExecuteDirect(m_wszQuery);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc left OJ   3 Tables
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_2()
{
 	swprintf(m_wszQuery,												 //short left OJ(3 tables)
			GetidsString(m_buf,idsInOutShortOJSyntax),
			m_pTable3->GetTableName(),
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			m_pTable3->GetTableName(),
			m_wszTable3ColName,
			L"=",
			g_pTable->GetTableName(),
			m_wszGlobalTableColName);
	
	return ExecuteDirect(m_wszQuery);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Left OJ   OJ_INNER  3 Tables
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_3()
{
	if(!IsSupportedOJ(SQL_OJ_INNER))
		return TEST_SKIPPED;

	swprintf(m_wszQuery,												 //short left OJ,OJ_INNER(3 tables)
			GetidsString(m_buf,idsInOutShortOJSyntax),
			m_pTable3->GetTableName(),
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			m_pTable3->GetTableName(),
			m_wszTable3ColName,
			L"=",
			g_pTable->GetTableName(),
			m_wszGlobalTableColName);
		
	return ExecuteDirect(m_wszQuery);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Left OJ and OJ_ALL_COMPARISION_OPS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_4()
{
	if(!IsSupportedOJ(SQL_OJ_ALL_COMPARISON_OPS))
		return TEST_SKIPPED;

 	swprintf(m_wszQuery,
			GetidsString(m_buf,idsShortLJoin),
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L">=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName);
	
	return ExecuteDirect(m_wszQuery);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Left OJ and SQL_OJ_NOT_ORDERED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_5()
{
	if(!IsSupportedOJ(SQL_OJ_NOT_ORDERED))
		return TEST_SKIPPED;

	swprintf(m_wszQuery,
			GetidsString(m_buf,idsShortLJoin),
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			L"=",
			g_pTable->GetTableName(),
			m_wszGlobalTableColName);
		
	return ExecuteDirect(m_wszQuery);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Right OJ
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_6()
{
	swprintf(m_wszQuery,
			GetidsString(m_buf,idsShortRJoin),
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName);
	
	return ExecuteDirect(m_wszQuery);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Right OJ and OJ_ALL_COMPARISION_OPS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_7()
{
	if(!IsSupportedOJ(SQL_OJ_ALL_COMPARISON_OPS))
		return TEST_SKIPPED;

	swprintf(m_wszQuery,
			GetidsString(m_buf,idsShortRJoin),
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"<=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName);

	return ExecuteDirect(m_wszQuery);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Right OJ and SQL_OJ_NOT_ORDERED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_8()
{
	if(!IsSupportedOJ(SQL_OJ_NOT_ORDERED))
		return TEST_SKIPPED;
    
	swprintf(m_wszQuery,
			GetidsString(m_buf,idsShortRJoin),
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			L"=",
			g_pTable->GetTableName(),
			m_wszGlobalTableColName);
	
	return ExecuteDirect(m_wszQuery);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Nested Outer Join
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_9()
{
	// SQL95 bug #11015: Nested OJ do not work on a 4.21a Server!
	if(!IsSupportedOJ(SQL_OJ_NESTED))
		return TEST_SKIPPED;

 	swprintf(m_wszQuery,
			GetidsString(m_buf,idsNestedShortJoin),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			m_pTable3->GetTableName(),
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			L"=",
			m_pTable3->GetTableName(),
			m_wszTable3ColName,
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			L"=",
			g_pTable->GetTableName(),
			m_wszGlobalTableColName);

	return ExecuteDirect(m_wszQuery);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Nested OJ and OJ_ALL_COMPARISION_OPS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_10()
{	
	if(!IsSupportedOJ(SQL_OJ_ALL_COMPARISON_OPS))
		return TEST_SKIPPED;
	
   	swprintf(m_wszQuery,
			GetidsString(m_buf,idsNestedShortJoin),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			m_pTable3->GetTableName(),
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			L"<=",
			m_pTable3->GetTableName(),
			m_wszTable3ColName,
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			L">=",
			g_pTable->GetTableName(),
			m_wszGlobalTableColName);

	return ExecuteDirect(m_wszQuery);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Left OJ with multiple join predicates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_11()
{ 
	static WCHAR s_wszOJ[] =	L"select * from {oj %s left outer join %s on %s.%s%s%s.%s and %s.%s%s%s.%s}";
	
	swprintf(m_wszQuery, 			
			s_wszOJ,
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName);
	
	return ExecuteDirect(m_wszQuery);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Right OK with multiple join predicates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_12()
{ 
	static WCHAR s_wszOJ[] =	L"select * from {oj %s right outer join %s on (%s.%s%s%s.%s) and (%s.%s%s%s.%s)}";

	swprintf(m_wszQuery, 			
			s_wszOJ,
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName);
	
	return ExecuteDirect(m_wszQuery);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Right OK with multiple join predicates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OuterJoins::Variation_13()
{ 
	static WCHAR s_wszOJ[] =	
		L"select * from {oj %s left outer join %s on %s.%s%s%s.%s and %s.%s%s%s.%s} where 1=1";
	
	swprintf(m_wszQuery, 			
			s_wszOJ,
			g_pTable->GetTableName(),
			m_pTable2->GetTableName(),
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName,
			g_pTable->GetTableName(),
			m_wszGlobalTableColName,
			L"=",
			m_pTable2->GetTableName(),
			m_wszTable2ColName);
	
	return ExecuteDirect(m_wszQuery);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL OuterJoins::Terminate()
{
	if (m_pTable2)
	{
		m_pTable2->DropTable();
		delete m_pTable2;
		m_pTable2=NULL;
	}

	if (m_pTable3)
	{
		m_pTable3->DropTable();
		delete m_pTable3;
		m_pTable3=NULL;
	}

	PROVIDER_FREE(m_wszGlobalTableColName);
	PROVIDER_FREE(m_wszTable2ColName);
	PROVIDER_FREE(m_wszTable3ColName);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LikeEscape)
//*-----------------------------------------------------------------------
//|	Test Case:		LikeEscape - Test Like Escape clause
//|	Created:			05/05/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LikeEscape::Init()
{
	WCHAR *		wszLiteralInsert = L"%a";
	DBORDINAL	ulOrdinal = 1;

	// Jet doesn't support the LIKE escape clause.
	if (g_ProvInfo.dwDBMSType == DRVR_QJET)
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		if (!CreateTypeTable(SQL_VARCHAR))
			return FALSE;
			
		if (!SUCCEEDED(m_p1TypeTable->InsertWithUserLiterals(1, &ulOrdinal, (WCHAR **)&(wszLiteralInsert))))
			return FALSE;

		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Canonical Escape
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LikeEscape::Variation_1()
{
	WCHAR *wszLiteralEscapePercent = L"\\";
	CCol	TempCol;

	if (!CHECK(m_p1TypeTable->GetColInfo(1, TempCol),S_OK))
		return TEST_FAIL;
	
	swprintf(m_wszQuery,L"select * from %s where %s like '%s%%a%%' {escape '%s'}", m_p1TypeTable->GetTableName(), TempCol.GetColName(), wszLiteralEscapePercent,wszLiteralEscapePercent);

	return CheckQry(L"%a",CHECKSTR);	
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LikeEscape::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LiteralDateTimeClauses)
//*-----------------------------------------------------------------------
//|	Test Case:		LiteralDateTimeClauses - Test literal date time escape clauses
//|	Created:			05/05/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LiteralDateTimeClauses::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc TimeStamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_1()
{					
	WCHAR	wszExpBuf[MAX_BUF];
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnTimestamp));

 	// Wont Fix SQL Server Bug AFB
 	if(g_ProvInfo.dwDBMSType == DRVR_SQLSRVR)
	{							 								
		if(g_ProvInfo.ulVersion > 6)
 			wcscpy(wszExpBuf,L"1994-12-12 01:12:56");
		else 
			wcscpy(wszExpBuf,L"19941212 01:12:56");
	}
	else 
 		wcscpy(wszExpBuf,L"1994-12-12 01:12:56");
		
	return CheckQry(wszExpBuf,CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_2()
{								   
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnDate));

	if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion >= 7)
		return CheckQry(L"1995-01-15 00:00:00",CHECKSTR);
	else if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion < 7)
		return CheckQry(L"19950115",CHECKSTR);
	else
		return CheckQry(L"1995-01-15",CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Time
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_3()
{																		   
	swprintf(m_wszQuery,g_wszScalarFmt,GetidsString(m_buf,idsFnTime));
	
	// TO DO - actually check the value
	// most provider append the current date to the time
	return CheckQry(L"IGNORE", CHECKSTR);  

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL YEAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_4()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL MONTH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_5()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL DAY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_6()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL HOUR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_7()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL MINUTE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_8()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL SECOND 3 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_9()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL YEAR TO MONTH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_10()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL DAY TO HOUR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_11()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL DAY TO MINUTE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_12()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL  DAY TO SECOND
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_13()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL  HOUR TO MINUTE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_14()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL  HOUR TO SECOND
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_15()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL  MINUTE TO SECOND
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_16()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc INTERVAL  DAY TO SECOND
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_17()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc GUID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralDateTimeClauses::Variation_18()
{ 
	// This canonical only supported with 3.x drivers
	if (g_ProvInfo.dwDBMSType == DRVR_ORACLE ||
		(g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion < 7) ||
		(g_ProvInfo.dwDBMSType == DRVR_QJET && g_ProvInfo.ulVersion < 4))
		return TEST_SKIPPED;

	swprintf(m_wszQuery,g_wszScalarFmt,L"guid '1A2B3C4D-AAAA-FFFF-1199-1234567890af'");

	if (g_ProvInfo.dwDBMSType == DRVR_QJET)
		return CheckQry(L"1A2B3C4D-AAAA-FFFF-1199-1234567890af", CHECKISTR);  
	else
		return CheckQry(L"{1A2B3C4D-AAAA-FFFF-1199-1234567890af}", CHECKISTR);  
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LiteralDateTimeClauses::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvertClause)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvertClause - Test canonical function convert
//|	Created:			05/05/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvertClause::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_INTEGER);		
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_BIGINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_1()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_BIGINT, L"9999", idsFnBIGINT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_BINARY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_2()
{
	// Expecting is "000" is a ported hack from ODBC
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_BINARY, L"111", idsFnBINARY, CHECKSTR, L"000");
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_BIT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_3()
{
	if (g_ProvInfo.dwDBMSType == DRVR_ORACLE)
		return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_BIT, L"0", idsFnBIT, CHECKSTR, L"0");
	else
		return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_BIT, L"0", idsFnBIT, CHECKSTR, L"False");
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_4()
{	
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_CHAR, L"32701", idsFnCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_DATE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_5()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_DATE, L"1299", idsFnDATE, CHECKSTR);

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_DECIMAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_6()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_DECIMAL, L"19999", idsFnDECIMAL, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_DOUBLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_7()
{		
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_DOUBLE, L"19199", idsFnDOUBLE, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_FLOAT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_8()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_FLOAT, L"99999", idsFnFLOAT, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_LONGVARBINARY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_9()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_LONGVARBINARY, L"11", idsFnLONGVARBINARY, CHECKSTR);

}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_10()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_LONGVARCHAR, L"-11234", idsFnLONGVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_NUMERIC
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_11()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_NUMERIC, L"-1", idsFnNUMERIC, CHECKFLOAT);

}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_REAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_12()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_REAL, L"12991", idsFnREAL, CHECKFLOAT);

}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_SMALLINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_13()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_SMALLINT, L"0", idsFnSMALLINT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_TIME
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_14()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_TIME, L"1212", idsFnTIME, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_TIMESTAMP
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_15()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_TIMESTAMP, L"9999", idsFnTIMESTAMP, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_TINYINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_16()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_TINYINT, L"127", idsFnTINYINT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_VARBINARY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_17()
{
	// Expecting is "000" is a ported hack from ODBC
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_VARBINARY, L"111", idsFnVARBINARY, CHECKSTR, L"000");
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Integer to SQL_VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_18()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_VARCHAR, L"12234", idsFnVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc To SQL_WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_19()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_WCHAR, L"-21000099", idsFnWCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc To SQL_VARWCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_20()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_WVARCHAR, L"+128", idsFnWVARCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc To SQL_WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertClause::Variation_21()
{
	return CheckConversion(SQL_CONVERT_INTEGER, SQL_CVT_WLONGVARCHAR, L"65536", idsFnWLONGVARCHAR, CHECKINT);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvertClause::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvertFromTim)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvertFromTim - Test conversion from TIME
//|	Created:			05/11/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvertFromTim::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_TIME);		
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc TO SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertFromTim::Variation_1()
{
	return CheckConversion(SQL_CONVERT_TIME, SQL_CVT_CHAR, L"12:59:59", idsFnCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc TO SQL_DATE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertFromTim::Variation_2()
{
	return CheckConversion(SQL_CONVERT_TIME, SQL_CVT_DATE, L"01:01:01", idsFnDATE, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To SQL_TIME
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertFromTim::Variation_3()
{
	return CheckConversion(SQL_CONVERT_TIME, SQL_CVT_TIME, L"01:01:01", idsFnTIME, CHECKSTR, L"01:01:01");
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To SQL_TIMESTAMP
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertFromTim::Variation_4()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvertFromTim::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvertStamp)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvertStamp - Test conversion from TIMESTAMP
//|	Created:			05/11/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvertStamp::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_TIMESTAMP);		
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_1()
{
	if (g_ProvInfo.dwDBMSType == DRVR_ORACLE)
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_CHAR, L"1904-02-21 12:59:59", idsFnCHAR, CHECKSTR, L"21-FEB-04");
	else if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion < 7)
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_CHAR, L"1904-02-21 12:59:59", idsFnCHAR, CHECKSTR, L"19040221 12:59:59", L"Feb 21 1904 12:59PM");
	else if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion >= 7)
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_CHAR, L"1904-02-21 12:59:59", idsFnCHAR, CHECKSTR, L"Feb 21 1904 12:59PM");
	else
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_CHAR, L"1904-02-21 12:59:59", idsFnCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To SQL_DATE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_2()
{
	return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_DATE, L"1904-02-21 12:59:59", idsFnDATE, CHECKSTR, L"1904-02-21");
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To SQL_TIME
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_3()
{
	return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_TIME, L"1904-02-21 12:59:59", idsFnTIME, CHECKSTR, L"12:59:59");
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To SQL_TIMESTAMP
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_4()
{
	return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_TIMESTAMP, L"1904-02-21 12:59:59", idsFnTIMESTAMP, CHECKSTR, L"1904-02-21 12:59:59");
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To SQL_VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_5()
{

	if (g_ProvInfo.dwDBMSType == DRVR_ORACLE)
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_VARCHAR, L"1904-02-21 12:59:59", idsFnVARCHAR, CHECKSTR, L"21-FEB-04");
	else if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion < 7)
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_VARCHAR, L"1904-02-21 12:59:59", idsFnVARCHAR, CHECKSTR, L"19040221 12:59:59", L"Feb 21 1904 12:59PM");
	else if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion >= 7)
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_CHAR, L"1904-02-21 12:59:59", idsFnCHAR, CHECKSTR, L"Feb 21 1904 12:59PM");
	else
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_VARCHAR, L"1904-02-21 12:59:59", idsFnVARCHAR, CHECKSTR);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To SQL_LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_6()
{
	return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_LONGVARCHAR, L"1904-02-21 12:59:59", idsFnLONGVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To SQL_VARWCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_7()
{
	if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion >= 7)
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_CHAR, L"2004-02-21 12:59:59", idsFnCHAR, CHECKSTR, L"Feb 21 2004 12:59PM");
	else
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_WVARCHAR, L"2004-02-21 12:59:59", idsFnWVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc To SQL_WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_8()
{
	if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion >= 7)
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_CHAR, L"2004-02-21 12:59:59", idsFnCHAR, CHECKSTR, L"Feb 21 2004 12:59PM");
	else
		return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_WCHAR, L"2004-02-21 12:59:59", idsFnWCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc To SQL_LONGVARWCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvertStamp::Variation_9()
{
	return CheckConversion(SQL_CONVERT_TIMESTAMP, SQL_CVT_WLONGVARCHAR, L"2004-02-21 12:59:59", idsFnWLONGVARCHAR, CHECKSTR);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvertStamp::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromLongVarBin)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromLongVarBin - Test conversions from SQL_LONGVARBINARY
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromLongVarBin::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_LONGVARBINARY);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To Binary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_1()
{
	return CheckConversion(SQL_CONVERT_LONGVARBINARY, SQL_CVT_BINARY, L"1111", idsFnBINARY, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To Char
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To LongVarBinary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_3()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To LongVarChar
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_4()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To varbinary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_5()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To Varchar
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_6()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To SQL_WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_7()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc To SQL_VARWCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_8()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc To SQL_LONGVARWCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarBin::Variation_9()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromLongVarBin::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromLongVarChar)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromLongVarChar - Test conversions from SQL_LONGVARCHAR
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromLongVarChar::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_LONGVARCHAR);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To Bigint
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_1()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To Binary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_2()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To Bit
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_3()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To Char
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_4()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To SQL_DATE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_5()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To SQL_DECIMAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_6()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To SQL_DOUBLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_7()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc To SQL_FLOAT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_8()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc To SQL_INTEGER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_9()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc To SQL_LONGVARBINARY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_10()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc To SQL_LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_11()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc To SQL_NUMERIC
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_12()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc To SQL_REAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_13()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc To SQL_SMALLINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_14()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc To SQL_TIME
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_15()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc To SQL_TIMESTAMP
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_16()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc To TinyInt
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_17()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc To VarBinary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_18()
{
	// just test the parser
	return CheckConversion(SQL_CONVERT_LONGVARCHAR, SQL_CVT_VARBINARY, L"44", idsFnVARBINARY, CHECKSTR, L"IGNORE");
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc To Varchar
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_19()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc To SQL_WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_20()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc To SQL_VARWCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_21()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc To SQL_LONGVARWCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromLongVarChar::Variation_22()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromLongVarChar::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromNumeric)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromNumeric - Test conversions from SQL_NUMERIC
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromNumeric::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_NUMERIC);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ToBigInt
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_1()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_CHAR, L"-9223372036854775808", idsFnCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To Binary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_2()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_CHAR, L"111", idsFnCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To Bit
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_3()
{
	// Hydra server AVs
	if (g_ProvInfo.dwDBMSType == DRVR_SQLSRVR && g_ProvInfo.ulVersion < 7)
		return TEST_SKIPPED;

	if (g_ProvInfo.dwDBMSType == DRVR_ORACLE)
		return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_BIT, L"1", idsFnBIT, CHECKSTR, L"0");  // Oracle doesn't support a backend SQL_BIT type		
	else
		return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_BIT, L"1", idsFnBIT, CHECKSTR, L"True");
		
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To Char
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_4()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_CHAR, L"12234", idsFnCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To SQL_DECIMAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_5()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_DECIMAL, L"18446744073709551616", idsFnDECIMAL, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To DOUBLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_6()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_DOUBLE, L"16777215", idsFnDOUBLE, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To FLOAT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_7()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_FLOAT, L"16777214", idsFnFLOAT, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc To INTEGER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_8()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_INTEGER, L"-2147483648", idsFnINTEGER, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc To LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_9()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_INTEGER, L"0", idsFnINTEGER, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc To NUMERIC
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_10()
{
	// special case Jet since they don't have genuine conversion support to NUMERIC
	if (g_ProvInfo.dwDBMSType == DRVR_QJET)
		return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_NUMERIC, L"922337203685477", idsFnNUMERIC, CHECKSTR);
	else
		return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_NUMERIC, L"18446744073709551615", idsFnNUMERIC, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc To REAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_11()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_REAL, L"-38001", idsFnREAL, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc To SMALLINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_12()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_SMALLINT, L"32767", idsFnSMALLINT, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc To TINYINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_13()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_TINYINT, L"11", idsFnTINYINT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc To VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_14()
{
return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_VARCHAR, L"1000000000", idsFnVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc TO WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_15()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_WCHAR, L"-38001", idsFnWCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc To WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_16()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_WVARCHAR, L"-38001", idsFnWVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc To WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromNumeric::Variation_17()
{
	return CheckConversion(SQL_CONVERT_NUMERIC, SQL_CVT_WLONGVARCHAR, L"-38001", idsFnWLONGVARCHAR, CHECKSTR);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromNumeric::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromReal)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromReal - Test conversion from SQL_REAL
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromReal::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_REAL);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_1()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_CHAR, L"27.123e+20", idsFnCHAR, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To DECIMAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_2()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_DECIMAL, L"+27.123e-20", idsFnDECIMAL, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To DOUBLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_3()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_DOUBLE, L"+127.1231e+36", idsFnDOUBLE, CHECKREAL);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To FLOAT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_4()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_FLOAT, L"1.6777214e+7", idsFnFLOAT, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To INTEGER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_5()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_INTEGER, L"30.465E+4", idsFnINTEGER, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To NUMERIC
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_6()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_NUMERIC, L"30.465E+4", idsFnNUMERIC, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To REAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_7()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_REAL, L"30.465E+4", idsFnREAL, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc To SMALLINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_8()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_SMALLINT, L"-30.46E+2", idsFnSMALLINT, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc To TINYINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_9()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_TINYINT, L"0.123456e+1", idsFnTINYINT, CHECKINT, L"1");
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc To VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_10()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_VARCHAR, L"56.7812e+24", idsFnVARCHAR, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc To LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_11()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_LONGVARCHAR, L"56.7812e+24", idsFnLONGVARCHAR, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc To WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_12()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_WCHAR, L"56.7812e+24", idsFnWCHAR, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc To WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_13()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_WVARCHAR, L"56.7812e+24", idsFnWVARCHAR, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc To WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromReal::Variation_14()
{
	return CheckConversion(SQL_CONVERT_REAL, SQL_CVT_WLONGVARCHAR, L"56.7812e+24", idsFnWLONGVARCHAR, CHECKFLOAT);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromReal::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromSmallInt)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromSmallInt - Test covnersion from SQL_SMALLINT
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromSmallInt::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_SMALLINT);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_1()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_SMALLINT, L"-31001", idsFnSMALLINT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To DECIMAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_2()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_DECIMAL, L"-31001", idsFnDECIMAL, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To DOUBLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_3()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_DOUBLE, L"-31001", idsFnDOUBLE, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To FLOAT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_4()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_FLOAT, L"-31001", idsFnFLOAT, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To INTEGER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_5()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_INTEGER, L"0", idsFnINTEGER, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To NUMERIC
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_6()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_NUMERIC, L"0", idsFnNUMERIC, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To REAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_7()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_REAL, L"1", idsFnREAL, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc To SMALLINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_8()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_SMALLINT, L"32767", idsFnSMALLINT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc To TINYINT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_9()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_TINYINT, L"127", idsFnTINYINT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc TO LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_10()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_LONGVARCHAR, L"0", idsFnLONGVARCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc To VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_11()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_VARCHAR, L"0", idsFnVARCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc to WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_12()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_WCHAR, L"0", idsFnWCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc To WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_13()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_WVARCHAR, L"1010", idsFnWVARCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc To WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromSmallInt::Variation_14()
{
	return CheckConversion(SQL_CONVERT_SMALLINT, SQL_CVT_WLONGVARCHAR, L"1010", idsFnWLONGVARCHAR, CHECKINT);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromSmallInt::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromTINYINT)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromTINYINT - Test conversions from SQL_TINYINT
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromTINYINT::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_TINYINT);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_1()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_CHAR, L"127", idsFnCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To DECIMAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_2()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_DECIMAL, L"0", idsFnDECIMAL, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To Double
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_3()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_DOUBLE, L"1", idsFnDOUBLE, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To Float
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_4()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_FLOAT, L"2", idsFnFLOAT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To Integer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_5()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_INTEGER, L"11", idsFnINTEGER, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To Numeric
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_6()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_NUMERIC, L"111", idsFnNUMERIC, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To Real
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_7()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_REAL, L"100", idsFnREAL, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc To LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_8()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_LONGVARCHAR, L"5", idsFnLONGVARCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc To Smallint
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_9()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_SMALLINT, L"51", idsFnSMALLINT, CHECKINT);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc To tinyint
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_10()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_TINYINT, L"99", idsFnTINYINT, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc To VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_11()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_VARCHAR, L"99", idsFnVARCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc To WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_12()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_WCHAR, L"99", idsFnWCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc To WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_13()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_WVARCHAR, L"99", idsFnWVARCHAR, CHECKINT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc To WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromTINYINT::Variation_14()
{
	return CheckConversion(SQL_CONVERT_TINYINT, SQL_CVT_WLONGVARCHAR, L"99", idsFnWLONGVARCHAR, CHECKINT);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromTINYINT::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromGUID)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromGUID - Test conversions from SQL_GUID
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromGUID::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_GUID);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To GUID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromGUID::Variation_1()
{
	return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_GUID, L"{00000001-0001-0001-0001-000000000001}", idsFnGUID, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromGUID::Variation_2()
{
	if(g_ProvInfo.dwDBMSType == DRVR_SQLSRVR)
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_CHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnCHAR, CHECKSTR, L"{00000001-0001-0001-0001-000000000001}", L"00000001-0001-0001-0001-000000000001");
	else
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_CHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromGUID::Variation_3()
{
	if(g_ProvInfo.dwDBMSType == DRVR_SQLSRVR)
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_VARCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnVARCHAR, CHECKSTR, L"{00000001-0001-0001-0001-000000000001}", L"00000001-0001-0001-0001-000000000001");
	else
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_VARCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromGUID::Variation_4()
{
	if(g_ProvInfo.dwDBMSType == DRVR_SQLSRVR)
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_LONGVARCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnLONGVARCHAR, CHECKSTR, L"{00000001-0001-0001-0001-000000000001}", L"00000001-0001-0001-0001-000000000001");
	else
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_LONGVARCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnLONGVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromGUID::Variation_5()
{
	if(g_ProvInfo.dwDBMSType == DRVR_SQLSRVR)
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_WCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnWCHAR, CHECKSTR, L"{00000001-0001-0001-0001-000000000001}", L"00000001-0001-0001-0001-000000000001");
	else
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_WCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnWCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromGUID::Variation_6()
{
	if(g_ProvInfo.dwDBMSType == DRVR_SQLSRVR)
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_WVARCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnWVARCHAR, CHECKSTR, L"{00000001-0001-0001-0001-000000000001}", L"00000001-0001-0001-0001-000000000001");
	else
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_WVARCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnWVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromGUID::Variation_7()
{
	if(g_ProvInfo.dwDBMSType == DRVR_SQLSRVR)
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_WLONGVARCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnWLONGVARCHAR, CHECKSTR, L"{00000001-0001-0001-0001-000000000001}", L"00000001-0001-0001-0001-000000000001");
	else
		return CheckConversion(SQL_CONVERT_GUID, SQL_CVT_WLONGVARCHAR, L"{00000001-0001-0001-0001-000000000001}", idsFnWLONGVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromGUID::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromWCHAR)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromWCHAR - Test conversion from SQL_WCHAR
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromWCHAR::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_WCHAR);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To BigInt
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_1()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_BIGINT, L"9223372036854775807", idsFnBIGINT, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc To Binary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_2()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_BINARY, L"1111", idsFnBINARY, CHECKSTR, L"3100310031003100");
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc To Bit
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_3()
{
	if (g_ProvInfo.dwDBMSType == DRVR_ORACLE)
		return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_BIT, L"1", idsFnBIT, CHECKSTR, L"1");
	else
		return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_BIT, L"1", idsFnBIT, CHECKSTR, L"True");
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc To Char
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_4()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_CHAR, L"1", idsFnCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc To SQL_DATE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_5()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_DATE, L"2000-02-29", idsFnDATE, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc To Decimal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_6()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_DECIMAL, L"1.9", idsFnDECIMAL, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc To Double
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_7()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_DOUBLE, L"1.912349", idsFnDOUBLE, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc To Float
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_8()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_FLOAT, L"1.9999", idsFnFLOAT, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc To Integer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_9()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_INTEGER, L"129", idsFnINTEGER, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc To LongVarBinary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_10()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_LONGVARBINARY, L"44", idsFnLONGVARBINARY, CHECKSTR, L"34003400");
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc To LongVarChar
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_11()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_LONGVARCHAR, L"1.1234", idsFnLONGVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc To Numeric
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_12()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_NUMERIC, L"129", idsFnNUMERIC, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc To Real
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_13()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_REAL, L"8388607", idsFnREAL, CHECKFLOAT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc To Smallint
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_14()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_SMALLINT, L"2", idsFnSMALLINT, CHECKSTR, L"2");
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc To Time
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_15()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc To TimeStamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_16()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc To TinyInt
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_17()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_TINYINT, L"127", idsFnTINYINT, CHECKSTR, L"127");
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc To VarBinary
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_18()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc To VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_19()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_VARCHAR, L"aZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZ", idsFnVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc To WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_20()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_WCHAR, L"aZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZ", idsFnWCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc To WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_21()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_WVARCHAR, L"aZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZ", idsFnWVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc To WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_22()
{
	return CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_WLONGVARCHAR, L"aZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZaZ", idsFnWLONGVARCHAR, CHECKSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc To SQL_GUID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromWCHAR::Variation_23()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromWCHAR::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromVarBin)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromVarBin - Test conversion from SQL_VARBINARY
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromVarBin::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_VARBINARY);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To VARBINARY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromVarBin::Variation_1()
{
	return CheckConversion(SQL_CONVERT_VARBINARY, SQL_CVT_VARBINARY, L"1111", idsFnVARBINARY, CHECKSTR, L"1111");
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc to CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromVarBin::Variation_2()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc to WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromVarBin::Variation_3()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromVarBin::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromVarChar)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromVarChar - Conversion from SQL_VARCHAR
//|	Created:			05/19/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromVarChar::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_VARCHAR);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To NUMERIC
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromVarChar::Variation_1()
{
	return CheckConversion(SQL_CONVERT_VARCHAR, SQL_CVT_NUMERIC, L"1.9", idsFnNUMERIC, CHECKSTR);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromVarChar::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ConvFromDecimal)
//*-----------------------------------------------------------------------
//|	Test Case:		ConvFromDecimal - Test conversion from SQL_DECIMAL
//|	Created:			05/22/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromDecimal::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		return CreateTypeTable(SQL_DECIMAL);
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc To CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ConvFromDecimal::Variation_1()
{
	return CheckConversion(SQL_CONVERT_DECIMAL, SQL_CVT_CHAR, L".1234567890123456789012345678", idsFnCHAR, CHECKFLOAT);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ConvFromDecimal::Terminate()
{
	DropTypeTable();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(Y2K_Tests)
//*-----------------------------------------------------------------------
//|	Test Case:		Y2K_Tests - Test timedate scalar funcs relating to Y2k
//| Created:  	7-8-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Y2K_Tests::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DAYNAME(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_1()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn DAYNAME({ts '2000-01-01 00:00:00'})");

	if(IsSupportedDateTime(SQL_FN_TD_DAYNAME))
		return CheckQry(L"Saturday",CHECKISTR);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DAYNAME(date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_2()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn DAYNAME({d '2000-02-29'})");

	if(IsSupportedDateTime(SQL_FN_TD_DAYNAME))
		return CheckQry(L"Tuesday",CHECKISTR);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DAYOFMONTH(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_3()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFMONTH({ts '2000-01-31 23:59:00'})");
		
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFMONTH))				
		return CheckQry(L"31",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DAYOFMONTH(date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_4()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFMONTH({d '2000-02-29'})");
		
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFMONTH))				
		return CheckQry(L"29",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_5()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK( {ts '2000-02-28 12:01:00'} )");

	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))
		return CheckQry(L"2", CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_6()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK( {d '2000-12-31'} )");

	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))
		return CheckQry(L"1", CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_7()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '2000-12-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"366",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_8()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '2001-12-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"365",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc MONTH(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_9()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn MONTH({ts '2000-02-29 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_MONTH))			
		return CheckQry(L"2",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc MONTH(date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_10()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn MONTH({d '2000-10-10'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_MONTH))			
		return CheckQry(L"10",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc QUARTER(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_11()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn QUARTER({ts '2000-03-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_QUARTER))			
		return 	CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc QUARTER(date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_12()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn QUARTER({d '2000-04-01'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_QUARTER))			
		return 	CheckQry(L"2",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc TIMESTAMPADD(timestamps
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_13()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn TIMESTAMPADD(SQL_TSI_HOUR, 13,{ts '1999-12-31 12:00:00'})");

	if(IsSupportedDateTime(SQL_FN_TD_TIMESTAMPADD))
		return CheckQry(L"2000-01-01 01:00:00.000",CHECKTIMESTAMP);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc TIMESTAMPADD(dates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_14()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn TIMESTAMPADD(SQL_TSI_DAY, 3, {d '2000-02-28'})");

	if(IsSupportedDateTime(SQL_FN_TD_TIMESTAMPADD))
		return CheckQry(L"2000-03-02 00:00:00.000",CHECKTIMESTAMP);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc TIMESTAMPDIFF(timestamps
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_15()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn TIMESTAMPDIFF(SQL_TSI_MONTH, {ts '2000-03-01 00:00:00'}, {ts '1999-10-01 00:00:00'})");
		
	if(IsSupportedDateTime(SQL_FN_TD_TIMESTAMPDIFF))			
		return CheckQry(L"-5",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc TIMESTAMPDIFF(dates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_16()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn TIMESTAMPDIFF(SQL_TSI_DAY, {d '2000-12-31'}, {d '2000-01-01'})");
		
	if(IsSupportedDateTime(SQL_FN_TD_TIMESTAMPDIFF))			
		return CheckQry(L"-365",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc WEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_17()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn WEEK({ts '2000-01-01 13:00:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_WEEK))			
		return CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc WEEK(dates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_18()
{ 
swprintf(m_wszQuery,g_wszScalarFmt,L"fn WEEK({d '2000-01-01'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_WEEK))			
		return CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc YEAR(timestamps
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_19()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn YEAR({ts '2000-01-01 00:00:00'})");

	if(IsSupportedDateTime(SQL_FN_TD_YEAR))				
		return CheckQry(L"2000",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc YEAR(dates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_20()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn YEAR({d '2000-12-31'})");

	if(IsSupportedDateTime(SQL_FN_TD_YEAR))				
		return CheckQry(L"2000",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc HOUR(timestamps
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_21()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn HOUR({ts '2000-02-29 17:00:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_HOUR))				
		return CheckQry(L"17",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc HOUR(date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_22()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn HOUR({ts '2000-02-29 00:00:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_HOUR))				
		return CheckQry(L"0",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_23()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '1900-01-01 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"1",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_24()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK({ts '1900-02-28 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))			
		return CheckQry(L"4",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_25()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK({ts '1900-03-01 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))			
		return CheckQry(L"5",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc WEEK(dates
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_26()
{ 
swprintf(m_wszQuery,g_wszScalarFmt,L"fn WEEK({d '1901-01-01'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_WEEK))			
		return CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_27()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '1928-12-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"366",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_28()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '1928-12-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"366",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_29()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '1998-12-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"365",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_30()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK({ts '1999-01-01 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))			
		return CheckQry(L"6",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc QUARTER(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_31()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn QUARTER({ts '2000-01-10 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_QUARTER))			
		return 	CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc QUARTER(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_32()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt,L"fn QUARTER({ts '2001-01-01 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_QUARTER))			
		return 	CheckQry(L"1",CHECKINT);
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_33()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '2048-12-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"366",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_34()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '2049-12-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"365",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_35()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK({ts '2001-01-01 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))			
		return CheckQry(L"2",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_36()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK({ts '2002-01-01 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))			
		return CheckQry(L"3",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_37()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK({ts '2004-02-29 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))			
		return CheckQry(L"1",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc DAYOFYEAR(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_38()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFYEAR({ts '2098-12-31 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFYEAR))			
		return CheckQry(L"365",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_39()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK({ts '2100-02-28 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))			
		return CheckQry(L"1",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc DAYOFWEEK(timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_40()
{ 
	swprintf(m_wszQuery,g_wszScalarFmt, L"fn DAYOFWEEK({ts '2100-03-01 23:59:00'})");
	
	if(IsSupportedDateTime(SQL_FN_TD_DAYOFWEEK))			
		return CheckQry(L"2",CHECKINT);			
	else
		return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Nested TIMESTAMPADD timestamp
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_41()
{
	swprintf(m_wszQuery,g_wszScalarFmt, 
		L"fn TIMESTAMPADD(SQL_TSI_DAY, {fn DAYOFMONTH({ts '2000-01-01 00:00:00'})}, {ts '2000-02-28 00:00:00'})");

	if(IsSupportedDateTime(SQL_FN_TD_TIMESTAMPADD))
		return CheckQry(L"2000-02-29 00:00:00.000",CHECKTIMESTAMP);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Nested TIMESTAMPADD date
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_42()
{
	swprintf(m_wszQuery,g_wszScalarFmt, 
		L"fn TIMESTAMPADD(SQL_TSI_DAY, {fn DAYOFMONTH({d '2000-01-01'})}, {d '2000-02-28'})");

	if(IsSupportedDateTime(SQL_FN_TD_TIMESTAMPADD))
		return CheckQry(L"2000-02-29 00:00:00.000",CHECKTIMESTAMP);
	else
		return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//--------------------------------------------------------------------
// @mfunc Time canonical literal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Y2K_Tests::Variation_43()
{
	WCHAR		wszExpectedDate[MAXBUFLEN];
	SYSTEMTIME	SystemTime;

 	if(g_ProvInfo.dwDBMSType != DRVR_SQLSRVR)
		return TEST_SKIPPED;

	GetLocalTime(&SystemTime);

	swprintf(wszExpectedDate, L"%.4d-%.2d-%.2d %s", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, L"07:15:26");

	swprintf(m_wszQuery,g_wszScalarFmt, L"fn convert({ t '07:15:26'}, SQL_TIMESTAMP)");

	
	return CheckQry(wszExpectedDate,CHECKTIMESTAMP);
	
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Y2K_Tests::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(EURO)
//*-----------------------------------------------------------------------
//|	Test Case:		EURO - Ansi Unicode conversion cases involving EURO symbol
//|	Created:			09/28/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL EURO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CScalar::Init())
	// }}
	{
		BOOL fNoEuro = FALSE;
		char szEuro[2];
		ULONG cchBytesWritten = 0;

		m_wszEuro[0] = 0x20AC;
		m_wszEuro[1] = 0x0000;

		// make sure that EURO is supported on this OS.
		// For example, a Japanese OS won't have EURO support.
		cchBytesWritten = WideCharToMultiByte(	CP_ACP,
												0,
												m_wszEuro, 
												-1,
												szEuro, 
												sizeof(szEuro),
												NULL, 
												&fNoEuro);
		COMPARE(cchBytesWritten, 2);  // one byte for EURO char, one byte for null terminator

		if (fNoEuro)
			return TEST_SKIPPED;
		else
			return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_CHAR to SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_1()
{
	if(TRUE != CreateTypeTable(SQL_CHAR)) 
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_CHAR, SQL_CVT_CHAR, m_wszEuro, idsFnCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_VARCHAR to SQL_VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_2()
{
	if(TRUE != CreateTypeTable(SQL_VARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_VARCHAR, SQL_CVT_VARCHAR, m_wszEuro, idsFnVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_LONGVARCHAR to SQL_LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_3()
{
	if(TRUE != CreateTypeTable(SQL_LONGVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_LONGVARCHAR, SQL_CVT_LONGVARCHAR, m_wszEuro, idsFnLONGVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WCHAR to SQL_WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_4()
{
	if(TRUE != CreateTypeTable(SQL_WCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_WCHAR, m_wszEuro, idsFnWCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WVARCHAR to SQL_WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_5()
{
	if(TRUE != CreateTypeTable(SQL_WVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WVARCHAR, SQL_CVT_WVARCHAR, m_wszEuro, idsFnWVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WLONGVARCHAR to SQL_WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_6()
{
	if(!CreateTypeTable(SQL_WLONGVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WLONGVARCHAR, SQL_CVT_WLONGVARCHAR, m_wszEuro, idsFnWLONGVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_CHAR to SQL_WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_7()
{
	if(TRUE != CreateTypeTable(SQL_CHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_CHAR, SQL_CVT_WCHAR, m_wszEuro, idsFnWCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_CHAR to SQL_WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_8()
{
	if(TRUE != CreateTypeTable(SQL_CHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_CHAR, SQL_CVT_WVARCHAR, m_wszEuro, idsFnWVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_CHAR to SQL_WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_9()
{
	if(TRUE != CreateTypeTable(SQL_CHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_CHAR, SQL_CVT_WLONGVARCHAR, m_wszEuro, idsFnWLONGVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_VARCHAR to SQLWCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_10()
{
	if(TRUE != CreateTypeTable(SQL_VARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_VARCHAR, SQL_CVT_WCHAR, m_wszEuro, idsFnWCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_VARCHAR to SQL_WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_11()
{
	if(TRUE != CreateTypeTable(SQL_VARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_VARCHAR, SQL_CVT_WVARCHAR, m_wszEuro, idsFnWVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_VARCHAR to SQL_WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_12()
{
	if(TRUE != CreateTypeTable(SQL_VARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_VARCHAR, SQL_CVT_WLONGVARCHAR, m_wszEuro, idsFnWLONGVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_LONGVARCHAR to SQL_WCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_13()
{
	if(TRUE != CreateTypeTable(SQL_LONGVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_LONGVARCHAR, SQL_CVT_WCHAR, m_wszEuro, idsFnWCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_LONGVARCHAR to SQL_WVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_14()
{
	if(TRUE != CreateTypeTable(SQL_LONGVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_LONGVARCHAR, SQL_CVT_WVARCHAR, m_wszEuro, idsFnWVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_LONGVARCHAR to SQL_WLONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_15()
{
	if(TRUE != CreateTypeTable(SQL_LONGVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_LONGVARCHAR, SQL_CVT_WLONGVARCHAR, m_wszEuro, idsFnWLONGVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WCHAR to SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_16()
{
	if(TRUE != CreateTypeTable(SQL_WCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_CHAR, m_wszEuro, idsFnCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WCHAR to SQL_VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_17()
{
	if(TRUE != CreateTypeTable(SQL_WCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_VARCHAR, m_wszEuro, idsFnVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WCHAR to SQL_LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_18()
{
	if(TRUE != CreateTypeTable(SQL_WCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WCHAR, SQL_CVT_LONGVARCHAR, m_wszEuro, idsFnLONGVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WVARCHAR to SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_19()
{
	if(TRUE != CreateTypeTable(SQL_WVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WVARCHAR, SQL_CVT_CHAR, m_wszEuro, idsFnCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WVARCHAR to SQL_VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_20()
{
	if(TRUE != CreateTypeTable(SQL_WVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WVARCHAR, SQL_CVT_VARCHAR, m_wszEuro, idsFnVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WVARCHAR to SQL_LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_21()
{
	if(TRUE != CreateTypeTable(SQL_WVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WVARCHAR, SQL_CVT_LONGVARCHAR, m_wszEuro, idsFnLONGVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WLONGVARCHAR to SQL_CHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_22()
{
	if(TRUE != CreateTypeTable(SQL_WLONGVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WLONGVARCHAR, SQL_CVT_CHAR, m_wszEuro, idsFnCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WLONGVARCHAR to SQL_VARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_23()
{
	if(TRUE != CreateTypeTable(SQL_WLONGVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WLONGVARCHAR, SQL_CVT_VARCHAR, m_wszEuro, idsFnVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Convert SQL_WLONGVARCHAR to SQL_LONGVARCHAR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EURO::Variation_24()
{
	if(TRUE != CreateTypeTable(SQL_WLONGVARCHAR))
	{
		DropTypeTable();
		return TEST_SKIPPED;
	}

	m_fPass = CheckConversion(SQL_CONVERT_WLONGVARCHAR, SQL_CVT_LONGVARCHAR, m_wszEuro, idsFnLONGVARCHAR, CHECKSTR);

	DropTypeTable();
	return m_fPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL EURO::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CScalar::Terminate());
}	// }}
// }}
// }}
