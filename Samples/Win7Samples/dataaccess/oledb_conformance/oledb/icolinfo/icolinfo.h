//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ICOLINFO.H | Header file for IColumnsInfo test module.
//
// @rev 01 | 06-10-95 | Microsoft | Created
// @rev 02 | 04-28-98 | Microsoft | Updated
//

#ifndef _ICOLINFO_H_
#define _ICOLINFO_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
		
//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define GETTEMPTABLEONLY(a,b,c,d)		PRVTRACE(L"***TEMP TABLE not invoked\n");
#define GETTEMPTABLEROWSET(a,b,c,d,e)	PRVTRACE(L"***TEMP TABLE not invoked\n");
#define FREETEMPTABLE(a)

#define INIT Init_Var();
#define TERM(x) if(x) return FALSE;
#define FREE Free();
#define CLEAR m_cColumns=0;m_rgInfo=NULL;

#define MAX_ROWS 100

// structure for building long data
struct LONG_DATA
{
	wchar_t *		pwszTypeName;
	unsigned short	dbType;
	unsigned int	uiColumnSize;
	wchar_t *		pwszLiteralPrefix;
	wchar_t *		pwszLiteralSuffix;
	wchar_t *		pwszCreateParams;
	VARIANT_BOOL	fIsNullable;
	VARIANT_BOOL	fCaseSensitive;
	unsigned int	uiSearchable;
	VARIANT_BOOL	fUnsignedAttribute;
	VARIANT_BOOL	fFixedPrecScale;
	VARIANT_BOOL	fAutoUniqueValue;
	wchar_t *		pwszLocalTypeName;
	short			iMinimumScale;
	short			iMaximumScale;
	GUID			gGuid;
	wchar_t *		pwszTypeLib;
	wchar_t *		pwszVersion;
	VARIANT_BOOL	fIsLong;
	VARIANT_BOOL	fBestMatch;
}; 

///////////////////////////////////////////////////////////////////////
//Enumerations
//
///////////////////////////////////////////////////////////////////////

//This enumeration represents various ways of obtaining a ROW 
//object. For e.g., TC_* represents the Test Case which will
//test a GetSession on a ROW object obtained from *.
enum ETESTCASE
{
	TC_Rowset = 1,				//Row from a Rowset
	TC_OpenRW,					//Row directly from OpenRowset call
	TC_Cmd,						//Row directly from command (singleton)
	TC_Bind,					//Row from direct binding thru Root Binder
	TC_IColInfo2,				//IColInfo2 on a row object
	TC_SingSel,						//This and TC_Cmd will be used for duplicating single select queries.
};

enum PREPARATION {SUPPORTED, NOTSUPPORTED, PREP_UNKNOWN};

enum METHOD_CHOICE
{
	INVALID_METHOD	= 0,
	GETCOLINFO		= INVALID_METHOD+1,
	MAPCOLID		= GETCOLINFO+1,
	GETRESCOLINFO	= MAPCOLID+1
};

enum ETXN
{
	ETXN_COMMIT,
	ETXN_ABORT
};

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR wszNOTIMPL[]= L"No current implementation.\n";

#define M 40
#define PROPERTIES 4 // bookmarks,deferred,cachedeferred,maywritecolumn

const WCHAR  wszTestFailure[]  =L"Variation could not run successfully due to prep failure\n";
const WCHAR  wszMaxPrecision[] =L"Maximum Precision exceeded";
const WCHAR  wszPropertySet[]  =L"WARNING: IDBProperties::GetPropertiesInfo says the property is readonly, but SetRowsetProperty succeeded.\n";

#endif 	//_ICOLINFO_H_
