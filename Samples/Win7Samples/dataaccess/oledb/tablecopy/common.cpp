//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module COMMON.CPP
//
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// Include
//
/////////////////////////////////////////////////////////////////////////////
#include "winmain.h"
#include "common.h"
#include "tablecopy.h"
#include "table.h"


/////////////////////////////////////////////////////////////////////////////
// 	Defines
//
/////////////////////////////////////////////////////////////////////////////

//Displays values like VALUE as   VALUE , L"VALUE"
#define VALUE_WCHAR(value) value, L#value

typedef struct _TYPEMAP
{
	DBTYPE		wType;				// The sql type value
	WCHAR*		pwszTypeName;		// Name for display
} TYPEMAP;

TYPEMAP rgDBTypes[] = 
{
	VALUE_WCHAR(NULL),
	VALUE_WCHAR(DBTYPE_I1),
	VALUE_WCHAR(DBTYPE_I2),
	VALUE_WCHAR(DBTYPE_I4),
	VALUE_WCHAR(DBTYPE_I8),
	VALUE_WCHAR(DBTYPE_UI1),
	VALUE_WCHAR(DBTYPE_UI2),
	VALUE_WCHAR(DBTYPE_UI4),
	VALUE_WCHAR(DBTYPE_UI8),
	VALUE_WCHAR(DBTYPE_R4),
	VALUE_WCHAR(DBTYPE_R8),
	VALUE_WCHAR(DBTYPE_CY),
	VALUE_WCHAR(DBTYPE_DECIMAL),
	VALUE_WCHAR(DBTYPE_NUMERIC),
	VALUE_WCHAR(DBTYPE_BOOL),
	VALUE_WCHAR(DBTYPE_ERROR),
	VALUE_WCHAR(DBTYPE_UDT),
	VALUE_WCHAR(DBTYPE_VARIANT),
	VALUE_WCHAR(DBTYPE_IDISPATCH),
	VALUE_WCHAR(DBTYPE_IUNKNOWN),
	VALUE_WCHAR(DBTYPE_GUID),
	VALUE_WCHAR(DBTYPE_DATE),
	VALUE_WCHAR(DBTYPE_DBDATE),
	VALUE_WCHAR(DBTYPE_DBTIME),
	VALUE_WCHAR(DBTYPE_DBTIMESTAMP),
	VALUE_WCHAR(DBTYPE_BSTR),
	VALUE_WCHAR(DBTYPE_STR),
	VALUE_WCHAR(DBTYPE_WSTR),
	VALUE_WCHAR(DBTYPE_BYTES),
};



/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToMBCS
//
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToMBCS(WCHAR* pwsz, CHAR* psz, ULONG cStrLen)
{
	ASSERT(pwsz && psz);

	//Convert the string to MBCS
	INT iResult = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, cStrLen, NULL, NULL);
	return iResult ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToWCHAR
//
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToWCHAR(CHAR* psz, WCHAR* pwsz, ULONG cStrLen)
{
	ASSERT(psz && pwsz);

	//Convert the string to MBCS
	INT iResult = MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, cStrLen);
	return iResult ? S_OK : E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
// WCHAR* wcsDuplicate
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* wcsDuplicate(WCHAR* pwsz)
{
	ASSERT(pwsz);
	
	//no-op case
	if(!pwsz)
		return NULL;
	
	size_t cLen	= wcslen(pwsz);

	//Allocate space for the string
	WCHAR* pwszBuffer = NULL;
	SAFE_ALLOC(pwszBuffer, WCHAR, cLen+1);

	//Now copy the string
	StringCchCopyW(pwszBuffer, cLen+1, pwsz);

CLEANUP:
	return pwszBuffer;
}


/////////////////////////////////////////////////////////////////////////////
// ULONG GetCreateParams
//
/////////////////////////////////////////////////////////////////////////////
ULONG GetCreateParams(WCHAR* pwszCreateParam)
{
	ASSERT(pwszCreateParam);
	
	ULONG	ulType = 0;								  
	
	if(wcsstr(pwszCreateParam, L"precision") || wcsstr(pwszCreateParam, L"PRECISION"))
		ulType |= CP_PRECISION;
	if(wcsstr(pwszCreateParam, L"scale") || wcsstr(pwszCreateParam, L"SCALE"))
		ulType |= CP_SCALE;
	if(wcsstr(pwszCreateParam, L"length") || wcsstr(pwszCreateParam, L"LENGTH"))
		ulType |= CP_LENGTH;
	if(wcsstr(pwszCreateParam, L"max length") || wcsstr(pwszCreateParam, L"MAX LENGTH"))
		ulType |= CP_MAXLENGTH;

	return ulType;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL IsVariableType
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsVariableType(DBTYPE wType)
{
	//According to OLEDB Spec Appendix A (Variable-Length Data Types)
	switch(wType) 
	{
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_BYTES:
			return TRUE;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL IsFixedType
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsFixedType(DBTYPE wType)
{
	return !IsVariableType(wType);
}

/////////////////////////////////////////////////////////////////////////////
// BOOL IsNumericType
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsNumericType(DBTYPE wType)
{
	//According to OLEDB Spec Appendix A (Numeric Data Types)
	switch(wType) 
	{
		case DBTYPE_I1:
		case DBTYPE_I2:
		case DBTYPE_I4:
		case DBTYPE_I8:
		case DBTYPE_UI1:
		case DBTYPE_UI2:
		case DBTYPE_UI4:
		case DBTYPE_UI8:
		case DBTYPE_R4:
		case DBTYPE_R8:
		case DBTYPE_CY:
		case DBTYPE_DECIMAL:
		case DBTYPE_NUMERIC:
			return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// WCHAR* GetDBTypeName
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* GetDBTypeName(DBTYPE wType)
{
	// Do a table look-up on the type
	for(ULONG i=0; i<NUMELE(rgDBTypes); i++)
	{
		if(wType == rgDBTypes[i].wType) 
			return rgDBTypes[i].pwszTypeName;
	}

	return rgDBTypes[0].pwszTypeName;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL GetPromotedType
//
/////////////////////////////////////////////////////////////////////////////
BOOL GetPromotedType(DBTYPE* pwType)
{
	ASSERT(pwType);

	//For fixed types we first consider promoting signed types
	//to unsigned types first, before promoting up to the 
	//next actual type.  Like DBTYPE_I1 -> DBTYPE_UI1,
	//instead of going directly to DBTYPE_I2.

	switch(*pwType) 
	{

		// Integer family
		case DBTYPE_BOOL:
			*pwType = DBTYPE_I1;
			break;

		case DBTYPE_I1:
			*pwType = DBTYPE_UI1;
			break;

		case DBTYPE_UI1:
			*pwType = DBTYPE_I2;
			break;
			
		case DBTYPE_I2:
			*pwType = DBTYPE_UI2;
			break;
				
		case DBTYPE_UI2:
			*pwType = DBTYPE_I4;
			break;
			
		case DBTYPE_I4:
			*pwType = DBTYPE_UI4;
			break;

		case DBTYPE_UI4:
		case DBTYPE_CY:
			*pwType = DBTYPE_I8;
			break;
		
		case DBTYPE_I8:
			*pwType = DBTYPE_UI8;
			break;

		case DBTYPE_UI8:
			*pwType = DBTYPE_NUMERIC;
			break;


		// Floating-point type family.  FLOAT and DOUBLE actually
		// have the same precision, so do a mutual promotion (that
		// is, allow FLOAT to become DOUBLE and DOUBLE to become
		// FLOAT) before going to CHAR.
		//
		case DBTYPE_R4:
			*pwType = DBTYPE_R8;
			break;
			
		// Fixed-point exact numerics. Ordering of the two types
		// is unimportant--for our purposes they have exactly the
		// same semantics.  
		//
		case DBTYPE_NUMERIC:
			*pwType = DBTYPE_DECIMAL;
			break;
			
		case DBTYPE_DECIMAL:
			*pwType = DBTYPE_R8;
			break;
						
		// Binary types
		case DBTYPE_BYTES:
			*pwType = DBTYPE_STR;
			break;

		// Date/Time family
		case DBTYPE_DATE:
		case DBTYPE_DBTIME:
		case DBTYPE_DBDATE:
			*pwType = DBTYPE_DBTIMESTAMP;
			break;
			
		case DBTYPE_R8:
		case DBTYPE_DBTIMESTAMP:
			*pwType = DBTYPE_BYTES;
			break;

		// Allow WSTR to STR.
		case DBTYPE_WSTR:
			*pwType = DBTYPE_STR;
			break;

		case DBTYPE_VARIANT:
			*pwType = DBTYPE_STR;
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

	

///////////////////////////////////////////////////////////////
// BOOL FreeBindingData
//
///////////////////////////////////////////////////////////////
BOOL FreeBindingData(ULONG cBindings, DBBINDING* rgBindings, void* pData)
{
	ASSERT(pData);
	
	//Need to walk the array and free any other alloc memory
	for(ULONG i=0; i<cBindings; i++)
	{
		//Free any "out-of-line" memory
		//VARIANT
		if(rgBindings[i].wType == DBTYPE_VARIANT)
		{
			VARIANT* pVariant = (VARIANT*)&BINDING_VALUE(rgBindings[i], pData);
			FreeVariants(1, pVariant);
		}

		//Free any pObjects
		SAFE_FREE(rgBindings[i].pObject);
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////
// BOOL FreeBindings
//
///////////////////////////////////////////////////////////////
BOOL FreeBindings(ULONG cBindings, DBBINDING* rgBindings)
{
	//Need to walk the array and free any other alloc memory
	for(ULONG i=0; i<cBindings; i++)
	{
		//Free any pObjects
		SAFE_FREE(rgBindings[i].pObject);
	}

	//Now we can free the outer struct
	SAFE_FREE(rgBindings);
	return TRUE;
}



///////////////////////////////////////////////////////////////
// Static Strings Messages
//
///////////////////////////////////////////////////////////////

extern WCHAR wsz_OLEDB[]				= L"Microsoft OLE DB TableCopy";
extern WCHAR wsz_SUCCESS[]				= L"Microsoft OLE DB TableCopy - Success";
extern WCHAR wsz_WARNING[]				= L"Microsoft OLE DB TableCopy - Warning";
extern WCHAR wsz_INFO[]					= L"Microsoft OLE DB TableCopy - Info";
extern WCHAR wsz_ERROR[]				= L"Microsoft OLE DB TableCopy - Error";
extern WCHAR wsz_CANCEL[]				= L"Microsoft OLE DB TableCopy - Cancel";
extern WCHAR wsz_ERRORINFO[]			= L"Microsoft OLE DB TableCopy - IErrorInfo";
								 
//Copying Status
extern WCHAR wsz_COPYING[] 				= L"Copying records";
extern WCHAR wsz_COPIED_RECORDS[]		= L"%Id records copied";
extern WCHAR wsz_COPY_SUCCESS[]			= L"Copy succeeded, %Id records copied!";
extern WCHAR wsz_COPY_FAILURE[]			= L"Copy failed!";
extern WCHAR wsz_CANCEL_OP[]			= L"Do you want to cancel?";
extern WCHAR wsz_TYPEMAPPING_FAILURE[]	= L"Mapping of Data Types Failed!";

//Tables
extern WCHAR wsz_CREATE_TABLE[]			= L"CREATE TABLE ";
extern WCHAR wsz_DROP_TABLE_[]			= L"DROP TABLE %s ";
extern WCHAR wsz_ASK_DROP_TABLE_[] 		= L"%s %s already exists.  Would you like to drop it?";
extern WCHAR wsz_SAME_TABLE_NAME[] 		= L"Target %s name must be different on the same DataSource";
extern WCHAR wsz_FROMTABLEHELP_[]		= L"Select Desired %s and Columns to Copy";
extern WCHAR wsz_FROMQUALTABLE_[]		= L"%s, %Id Column(s)";
extern WCHAR wsz_TOTABLEHELP_[]			= L"Select Target %s Name";

//Indexes
extern WCHAR wsz_CREATE_INDEX_[]		= L"CREATE%sINDEX ";
extern WCHAR wsz_UNIQUE_INDEX[]			= L" UNIQUE ";
extern WCHAR wsz_INDEX_DESC[] 			= L" DESC ";
extern WCHAR wsz_INDEX_FAILED_[]		= L"INDEX %s failed to be created.  Would you like to Continue?";

//Columns
extern WCHAR wsz_NO_TYPE_MATCH_[]		= L"Target Data type not found for %s type";
extern WCHAR wsz_NO_TYPE_FOUND_[]		= L"Source Data type not found for %s type";

extern WCHAR wsz_CONNECT_STRING_[]		= L"%s    %s    %s %s    %s %s";
extern WCHAR wsz_TYPES_STRING_[]		= L"%s    %s    %s=%s";
extern WCHAR wsz_NOT_CONNECTED[]		= L"Not Connected: Press Connect to establish a connection";

extern WCHAR wsz_PROVIDER_INFO_[]		= L"%s";

extern WCHAR wsz_INVALID_VALUE_[]		= L"Invalid Value '%s' specified.  Please specify a value >= %lu and <= %lu.";
extern WCHAR wsz_READONLY_DATASOURCE_[]	= L"Datasource %s is marked as read only, it may not be updateable.";

//Query
extern WCHAR wsz_SHOW_SQL_[]			= L"DSN = %s\n\nSQL = %s";
extern WCHAR wsz_SELECT[]				= L" SELECT ";
extern WCHAR wsz_FROM[]					= L" FROM ";
extern WCHAR wsz_BOGUS_WHERE[]			= L" WHERE 0 = 1";
extern WCHAR wsz_INSERT_INTO[]			= L"INSERT INTO ";
extern WCHAR wsz_VALUES_CLAUSE[]		= L" ) VALUES ( ";
extern WCHAR wsz_PARAM[]				= L"?";
extern WCHAR wsz_PRIMARY_KEY[]			= L" PRIMARY KEY";

//General String Values
extern WCHAR wsz_COMMA[]				= L", ";
extern WCHAR wsz_LPAREN[]				= L" ( ";
extern WCHAR wsz_RPAREN[]				= L" ) ";
extern WCHAR wsz_SPACE[]				= L" ";
extern WCHAR wsz_PERIOD[]				= L".";
