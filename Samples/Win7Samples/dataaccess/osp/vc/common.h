//--------------------------------------------------------------------
// Microsoft OLE DB Test OLEDB Simple Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation.  All Rights Reserved.
//
// @doc
//
// @module COMMON.H | Assertion Routines
//
//

////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////

//	OLE DB headers
#include "oledb.h"
#include "oledberr.h"

#include "msdaosp.h"		//OLEDBSimpleProvider
#include <ocidl.h>			//IConnectionPointContainer


////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////
//IMalloc Wrappers
#define CHECK_MEMORY(pv)			if(!(pv))  goto CLEANUP

#define SAFE_ALLOC(pv, type, cb)	{ pv = (type*)CoTaskMemAlloc((cb)*sizeof(type)); CHECK_MEMORY(pv); }
#define SAFE_REALLOC(pv, type, cb)	{ pv = (type*)CoTaskMemRealloc(pv, (cb)*sizeof(type)); CHECK_MEMORY(pv);}
#define SAFE_SYSALLOC(pv, bstr)		{ pv = SysAllocString(bstr); CHECK_MEMORY(pv); }

#define SAFE_FREE(pv)				{ CoTaskMemFree(pv); pv = NULL; }
#define SAFE_SYSFREE(bstr)			{ SysFreeString(bstr); bstr = NULL;}

//IUnknown->Release Wrapper
#define SAFE_RELEASE(pv)			if((pv)) { (pv)->Release(); (pv) = NULL; }  
#define SAFE_ADDREF(pv)				if((pv)) { (pv)->AddRef();				}  
																									
//Test macros																					
#define TEST(exp)					{ if(FAILED(exp)) { ASSERT(!#exp); } }
#define TESTC(exp)					{ if(FAILED(exp)) { goto CLEANUP;  } }

#define NUMELEM(x)		(sizeof(x)/sizeof(*x))

#define MAX_NAME_LEN	 256
#define MAX_BLOCK_SIZE	  50

#define EQUAL		 (0)
#define LESSTHAN	(-1)
#define GREATERTHAN  (1)
#define NOTEQUAL	(-2) 

////////////////////////////////////////////////////////
// General
//
////////////////////////////////////////////////////////
// for VT_BSTR and some other type, relationship will also be given
LONG CompareVariant(VARIANT* pVar1, VARIANT* pVar2,	BOOL fCaseSensitive);


////////////////////////////////////////////////////////
// RegEntry
//
////////////////////////////////////////////////////////
struct REGENTRY
{
	HKEY  hRootKey;
    CHAR* szKeyName;
    CHAR* szValueName;
    CHAR* szValue;
};

HRESULT GetRegEntry(REGENTRY* pRegEntry, CHAR* pszData, ULONG cBytes);
HRESULT GetRegEntry(HKEY hRootKey, CHAR* pszKeyName, CHAR* pszValueName, CHAR* pszValue, ULONG cBytes);

HRESULT SetRegEntry(REGENTRY* pRegEntry);
HRESULT SetRegEntry(HKEY hRootKey, CHAR* pszKeyName, CHAR* pszValueName, CHAR* pszValue);

HRESULT DelRegEntry(REGENTRY* pRegEntry);
HRESULT DelRegEntry(HKEY hRootKey, CHAR* pszKeyName);

HRESULT ConvertToMBCS(WCHAR* pwsz, CHAR* psz, ULONG cbStrLen);
HRESULT ConvertToWCHAR(CHAR* psz, WCHAR* pwsz, ULONG cbStrLen);

extern const IID IID_DataSourceListener;

//extern const IID IID_DataSource;

extern const IID IID_OLEDBSimpleProviderListener;

extern const IID IID_OLEDBSimpleProvider;

