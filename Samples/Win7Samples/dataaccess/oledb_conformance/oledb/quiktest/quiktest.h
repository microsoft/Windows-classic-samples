//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1997-2000 Microsoft Corporation
//
// @doc
//
// @module QUIKTEST.H | This module contains header information for 
//						OLE DB Provider Quik Test
//
//

#ifndef _QUIKTEST_H_
#define _QUIKTEST_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"

//----------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------
#define	FILENAME_SIZE	_MAX_FNAME
#define	PATH_SIZE		_MAX_PATH
#define MIN_ROWS		5          //Minimum number of rows on Table. 

//----------------------------------------------------------------------
// String constants
//----------------------------------------------------------------------
//These are used in the CBaseError class implementation.
const WCHAR	wszErrorFindingCurrentPath[] 	= L"Error finding current path.\n";
const WCHAR wszErrorTitle[]			= L"--- Error ----------\n";
const WCHAR wszWarningTitle[]		= L"--- Warning --------\n";


//----------------------------------------------------------------------
// Const Static Arrays
//----------------------------------------------------------------------
//Mandatory columns of a columns rowset.
struct tagMandCols
{
	const DBID*	pDbid;
	DBTYPE	wType;
	WCHAR*	pwszName;
} g_rgMandCols[] = 
{
	// DBID                 DBTYPE              Name
	&DBCOLUMN_IDNAME,		DBTYPE_WSTR,		L"DBCOLUMN_IDNAME",
	&DBCOLUMN_GUID,			DBTYPE_GUID,		L"DBCOLUMN_GUID",
	&DBCOLUMN_PROPID,		DBTYPE_UI4,			L"DBCOLUMN_PROPID",
	&DBCOLUMN_NAME,			DBTYPE_WSTR,		L"DBCOLUMN_NAME",
	&DBCOLUMN_NUMBER,		DBTYPEFOR_DBORDINAL,			L"DBCOLUMN_NUMBER",
	&DBCOLUMN_TYPE,			DBTYPE_UI2,			L"DBCOLUMN_TYPE",
	&DBCOLUMN_TYPEINFO,		DBTYPE_IUNKNOWN,	L"DBCOLUMN_TYPEINFO",
	&DBCOLUMN_COLUMNSIZE,	DBTYPEFOR_DBLENGTH,			L"DBCOLUMN_COLUMNSIZE",
	&DBCOLUMN_PRECISION,	DBTYPE_UI2,			L"DBCOLUMN_PRECISION",
	&DBCOLUMN_SCALE,		DBTYPE_I2,			L"DBCOLUMN_SCALE",
	&DBCOLUMN_FLAGS,		DBTYPE_UI4,			L"DBCOLUMN_FLAGS"
};

const ULONG g_cMandCols = NUMELEM(g_rgMandCols);

//List of Keywords from OLE DB.
const LPWSTR g_rgpwszKeywords[] = {
		L"ABSOLUTE",L"ACTION",L"ADD",L"ALL",L"ALLOCATE",L"ALTER",L"AND",L"ANY",L"ARE",L"AS",L"ASC",
		L"ASSERTION",L"AT",L"AUTHORIZATION",L"AVG",L"BEGIN",L"BETWEEN",L"BIT",L"BIT_LENGTH",L"BOTH",
		L"BY",L"CASCADE",L"CASCADED",L"CASE",L"CAST",L"CATALOG",L"CHAR",L"CHAR_LENGTH",L"CHARACTER",
		L"CHARACTER_LENGTH",L"CHECK",L"CLOSE",L"COALESCE",L"COLLATE",L"COLLATION",L"COLUMN",
		L"COMMIT",L"CONNECT",L"CONNECTION",L"CONSTRAINT",L"CONSTRAINTS",L"CONTINUE",L"CONVERT",
		L"CORRESPONDING",L"COUNT",L"CREATE",L"CROSS",L"CURRENT",L"CURRENT_DATE",L"CURRENT_TIME",
		L"CURRENT_TIMESTAMP",L"CURRENT_USER",L"CURSOR",L"DATE",L"DAY",L"DEALLOCATE",L"DEC",
		L"DECIMAL",L"DECLARE",L"DEFAULT",L"DEFERRABLE",L"DEFERRED",L"DELETE",L"DESC",L"DESCRIBE",
		L"DESCRIPTOR",L"DIAGNOSTICS",L"DISCONNECT",L"DISTINCT",L"DISTINCTROW",L"DOMAIN",L"DOUBLE",
		L"DROP",L"ELSE",L"END",L"END-EXEC",L"ESCAPE",L"EXCEPT",L"EXCEPTION",L"EXEC",L"EXECUTE",
		L"EXISTS",L"EXTERNAL",L"EXTRACT",L"FALSE",L"FETCH",L"FIRST",L"FLOAT",L"FOR",L"FOREIGN",
		L"FOUND",L"FROM",L"FULL",L"GET",L"GLOBAL",L"GO",L"GOTO",L"GRANT",L"GROUP",L"HAVING",L"HOUR",
		L"IDENTITY",L"IMMEDIATE",L"IN",L"INDICATOR",L"INITIALLY",L"INNER",L"INPUT",L"INSENSITIVE",
		L"INSERT",L"INT",L"INTEGER",L"INTERSECT",L"INTERVAL",L"INTO",L"IS",L"ISOLATION",L"JOIN",
		L"KEY",L"LANGUAGE",L"LAST",L"LEADING",L"LEFT",L"LEVEL",L"LIKE",L"LOCAL",L"LOWER",L"MATCH",L"MAX",
		L"MIN",L"MINUTE",L"MODULE",L"MONTH",L"NAMES",L"NATIONAL",L"NATURAL",L"NCHAR",L"NEXT",L"NO",
		L"NOT",L"NULL",L"NULLIF",L"NUMERIC",L"OCTET_LENGTH",L"OF",L"ON",L"ONLY",L"OPEN",L"OPTION",
		L"OR",L"ORDER",L"OUTER",L"OUTPUT",L"OVERLAPS",L"PARTIAL",L"POSITION",L"PRECISION",L"PREPARE",
		L"PRESERVE",L"PRIMARY",L"PRIOR",L"PRIVILEGES",L"PROCEDURE",L"PUBLIC",L"READ",L"REAL",
		L"REFERENCES",L"RELATIVE",L"RESTRICT",L"REVOKE",L"RIGHT",L"ROLLBACK",L"ROWS",L"SCHEMA",
		L"SCROLL",L"SECOND",L"SECTION",L"SELECT",L"SESSION",L"SESSION_USER",L"SET",L"SIZE",L"SMALLINT",
		L"SOME",L"SQL",L"SQLCODE",L"SQLERROR",L"SQLSTATE",L"SUBSTRING",L"SUM",L"SYSTEM_USER",L"TABLE",
		L"TEMPORARY",L"THEN",L"TIME",L"TIMESTAMP",L"TIMEZONE_HOUR",L"TIMEZONE_MINUTE",L"TO",
		L"TRAILING",L"TRANSACTION",L"TRANSLATE",L"TRANSLATION",L"TRIGGER",L"TRIM",L"TRUE",L"UNION",
		L"UNIQUE",L"UNKNOWN",L"UPDATE",L"UPPER",L"USAGE",L"USER",L"USING",L"VALUE",L"VALUES",L"VARCHAR",
		L"VARYING",L"VIEW",L"WHEN",L"WHENEVER",L"WHERE",L"WITH",L"WORK",L"WRITE",L"YEAR",L"ZONE",
};

//LITERALS Constants.
const LPWSTR g_rgpwszLiterals[] = {
		L"DBLITERAL_INVALID",L"DBLITERAL_BINARY_LITERAL",L"DBLITERAL_CATALOG_NAME",
		L"DBLITERAL_CATALOG_SEPARATOR",L"DBLITERAL_CHAR_LITERAL",
		L"DBLITERAL_COLUMN_ALIAS",L"DBLITERAL_COLUMN_NAME",L"DBLITERAL_CORRELATION_NAME",
		L"DBLITERAL_CURSOR_NAME",L"DBLITERAL_ESCAPE_PERCENT_PREFIX",L"DBLITERAL_ESCAPE_UNDERSCORE_PREFIX",
		L"DBLITERAL_INDEX_NAME",L"DBLITERAL_LIKE_PERCENT",L"DBLITERAL_LIKE_UNDERSCORE",
		L"DBLITERAL_PROCEDURE_NAME",L"DBLITERAL_QUOTE_PREFIX",L"DBLITERAL_SCHEMA_NAME",
		L"DBLITERAL_TABLE_NAME",L"DBLITERAL_TEXT_COMMAND",L"DBLITERAL_USER_NAME",
		L"DBLITERAL_VIEW_NAME",L"DBLITERAL_CUBE_NAME",L"DBLITERAL_DIMENSION_NAME",
		L"DBLITERAL_HIERARCHY_NAME",L"DBLITERAL_LEVEL_NAME",L"DBLITERAL_MEMBER_NAME",
		L"DBLITERAL_PROPERTY_NAME",L"DBLITERAL_SCHEMA_SEPARATOR",L"DBLITERAL_QUOTE_SUFFIX",
		L"DBLITERAL_ESCAPE_PERCENT_SUFFIX",L"DBLITERAL_ESCAPE_UNDERSCORE_SUFFIX"
};



///////////////////////////////////////////////////////////////////////
// CBaseError Class
//
///////////////////////////////////////////////////////////////////////
class CBaseError : public IError
{
public:
	DWORD					m_cRef;

	HRESULT					m_ExpectedHr;	
	HRESULT					m_ActualHr;		

	DWORD					m_cModErrors;
	DWORD					m_cCaseErrors;
	DWORD					m_cVarErrors;	
							
	DWORD					m_cModWarnings;
	DWORD					m_cCaseWarnings;
	DWORD					m_cVarWarnings;	
					
	ERRORLEVEL				m_ErrorLevel;

	CBaseError(void);
	virtual ~CBaseError();
	HRESULT Transmit(CHAR* pszString);


//
// IError
//

	STDMETHODIMP GetErrorLevel		(ERRORLEVEL *pErrorLevel);
	STDMETHODIMP SetErrorLevel		(ERRORLEVEL ErrorLevel);
	STDMETHODIMP GetActualHr		(LONG *phrActual);
	STDMETHODIMP Validate			(LONG hrActual, 
									 BSTR bstrFileName, 
									 LONG lLineNo, 
									 LONG hrExpected, 
									 VARIANT_BOOL *pfResult);
	STDMETHODIMP Compare			(VARIANT_BOOL fWereEqual, 
									 BSTR bstrFileName, 
									 LONG lLineNo, 
									 VARIANT_BOOL *pfWereEqual);
	STDMETHODIMP LogExpectedHr		(LONG hrExpected );
	STDMETHODIMP LogReceivedHr		(LONG hrReceived, 
									 BSTR bstrFileName, 
									 LONG lLineNo);
	STDMETHODIMP ResetModErrors		(void);
	STDMETHODIMP ResetModWarnings	(void);
	STDMETHODIMP ResetCaseErrors	(void);
	STDMETHODIMP ResetCaseWarnings	(void);
	STDMETHODIMP ResetVarErrors		(void);
	STDMETHODIMP ResetVarWarnings	(void);
	STDMETHODIMP GetModErrors		(LONG *plModErrors);
	STDMETHODIMP GetModWarnings		(LONG *plModWarnings);
	STDMETHODIMP GetCaseErrors		(LONG *plCaseErrors);
	STDMETHODIMP GetCaseWarnings	(LONG *plCaseWarnings);
	STDMETHODIMP GetVarErrors		(LONG *plVarErrors);
	STDMETHODIMP GetVarWarnings		(LONG *plVarWarnings);
	STDMETHODIMP Increment			(void);
	STDMETHODIMP Transmit			(BSTR bstrTextString);
	STDMETHODIMP Initialize			(void);

//
// IUnknown
//

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);
	STDMETHODIMP_(DWORD) AddRef(void);
	STDMETHODIMP_(DWORD) Release(void);
};	

#endif  //#ifndef _QUIKTEST_H_
