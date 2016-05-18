//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module COMMON.H
//
//-----------------------------------------------------------------------------

#ifndef _COMMON_H_
#define _COMMON_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "winmain.h"
#include "strsafe.h"

#include "oledb.h"
#include "oledberr.h"

#include "error.h"
#include "tablecopy.h"


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
#define NUMELE(rgEle) (sizeof(rgEle) / sizeof(rgEle[0]))
#define __WIDESTRING(str) L##str
#define WIDESTRING(str) __WIDESTRING(str)

//FindLeaks
#ifdef _DEBUG
#define DISP_FILE_LINE	TRACE("File '%s', Line '%lu'\n", __FILE__, __LINE__)
#else
#define DISP_FILE_LINE
#endif //_DEBUG

//IMalloc Wrappers
#define SAFE_ALLOC(pv, type, cb)	{ DISP_FILE_LINE; pv = (type*)CoTaskMemAlloc((cb)*sizeof(type)); CHECK_MEMORY(pv);			}
#define SAFE_REALLOC(pv, type, cb)	{ DISP_FILE_LINE; pv = (type*)CoTaskMemRealloc(pv, (cb)*sizeof(type)); CHECK_MEMORY(pv);	}
#define SAFE_SYSALLOC(pv, bstr)		{ pv = SysAllocString(bstr); CHECK_MEMORY(pv);												}		

#define SAFE_FREE(pv)				{ CoTaskMemFree(pv); pv = NULL;						}
#define SAFE_SYSFREE(bstr)			{ SysFreeString(bstr); bstr = NULL;					}

//IUnknown->Release Wrapper
#define SAFE_ADDREF(pv)				if(pv) { ((IUnknown*)(pv))->AddRef();				}
#define SAFE_RELEASE(pv)			if(pv) { ((IUnknown*)(pv))->Release(); pv = NULL;	}  
																									
//Test macros																					
#define CHECKC(exp)					{ if(!(exp)) { ASSERT(!#exp); goto CLEANUP;			} }
#define QTESTC(exp)					{ if(FAILED(exp)) goto CLEANUP;						}

#define XTEST(exp)					{ HRESULT Internalhr = exp;	if(FAILED(Internalhr)) { DisplayAllErrors(NULL, Internalhr, WIDESTRING(__FILE__), __LINE__); }}
#define XTESTC(exp)					{ HRESULT Internalhr = exp;	if(FAILED(Internalhr)) { DisplayAllErrors(NULL, Internalhr, WIDESTRING(__FILE__), __LINE__); goto CLEANUP; }}



///////////////////////////////////////////////////////////////////
// Accessor / Binding 
//
///////////////////////////////////////////////////////////////////
#define ADJUST_SIZE(Size, MaxSize)  ((Size) = min((Size), (MaxSize)))

//STATUS helpers, for locating obStatus offsets in the bindings
#define STATUS_IS_BOUND(Binding)    ( Binding.dwPart & DBPART_STATUS )
#define BINDING_STATUS(Binding, pv) (*(ULONG*)((BYTE*)pv + Binding.obStatus))

//LENGTH helpers, for locating obLength offsets in the bindings
#define LENGTH_IS_BOUND(Binding)    ( Binding.dwPart & DBPART_LENGTH )
#define BINDING_LENGTH(Binding, pv) (*(ULONG*)((BYTE*)pv + Binding.obLength))

//VALUE helpers, for locating obValue offsets in the bindings
#define VALUE_IS_BOUND(Binding)     ( Binding.dwPart & DBPART_VALUE )
#define BINDING_VALUE(Binding, pv)  (*(ULONG*)((BYTE*)pv + Binding.obValue ))

//ROUNDUP on all platforms pointers must be aligned properly
#define ROUNDUP_AMOUNT	8
#define ROUNDUP_(size,amount)		(((DBBYTEOFFSET)(size)+((amount)-1))&~((DBBYTEOFFSET)(amount)-1))
#define ROUNDUP(size)				ROUNDUP_(size, ROUNDUP_AMOUNT)

BOOL FreeBindings(ULONG cBindings, DBBINDING* rgBindings);
BOOL FreeBindingData(ULONG cBindings, DBBINDING* rgBindings, void* pData);



////////////////////////////////////////////////////////////////////////////
// DBTYPE functions
//
////////////////////////////////////////////////////////////////////////////
BOOL IsFixedType(DBTYPE wType);
BOOL IsVariableType(DBTYPE wType);
BOOL IsNumericType(DBTYPE wType);

WCHAR* GetDBTypeName(DBTYPE wType);
BOOL GetPromotedType(DBTYPE* pwType);
			  


////////////////////////////////////////////////////////////////////////////
// OLEDB General Helper functions
//
////////////////////////////////////////////////////////////////////////////
ULONG GetCreateParams(WCHAR* pwszCreateParam);

HRESULT ConvertToMBCS(WCHAR* pwsz, CHAR* psz, ULONG cbStrLen);
HRESULT ConvertToWCHAR(CHAR* psz, WCHAR* pwsz, ULONG cbStrLen);
WCHAR* wcsDuplicate(WCHAR* pwsz);


///////////////////////////////////////////////////////////////
// Static Strings Messages
//
///////////////////////////////////////////////////////////////

//General Statss
extern WCHAR wsz_OLEDB[];				
extern WCHAR wsz_SUCCESS[];				
extern WCHAR wsz_WARNING[];				
extern WCHAR wsz_INFO[];
extern WCHAR wsz_ERROR[];				
extern WCHAR wsz_CANCEL[];				
extern WCHAR wsz_ERRORINFO[];			
								 
//Copying Status
extern WCHAR wsz_COPYING[]; 				
extern WCHAR wsz_COPIED_RECORDS[];		
extern WCHAR wsz_COPY_SUCCESS[];			
extern WCHAR wsz_COPY_FAILURE[];			
extern WCHAR wsz_CANCEL_OP[];			
extern WCHAR wsz_TYPEMAPPING_FAILURE[];

//Tables
extern WCHAR wsz_CREATE_TABLE[];			
extern WCHAR wsz_DROP_TABLE_[];			
extern WCHAR wsz_ASK_DROP_TABLE_[];			
extern WCHAR wsz_SAME_TABLE_NAME[];
extern WCHAR wsz_FROMTABLEHELP_[];
extern WCHAR wsz_FROMQUALTABLE_[];
extern WCHAR wsz_TOTABLEHELP_[];

//Indexes
extern WCHAR wsz_CREATE_INDEX_[];		
extern WCHAR wsz_UNIQUE_INDEX[];			
extern WCHAR wsz_INDEX_DESC[]; 			
extern WCHAR wsz_INDEX_FAILED_[];		

//Columns
extern WCHAR wsz_NO_TYPE_MATCH_[];		
extern WCHAR wsz_NO_TYPE_FOUND_[];		

extern WCHAR wsz_CONNECT_STRING_[];
extern WCHAR wsz_TYPES_STRING_[];
extern WCHAR wsz_NOT_CONNECTED[];

extern WCHAR wsz_PROVIDER_INFO_[];

extern WCHAR wsz_INVALID_VALUE_[];	
extern WCHAR wsz_READONLY_DATASOURCE_[];	

//Query
extern WCHAR wsz_SHOW_SQL_[];
extern WCHAR wsz_SELECT[];				
extern WCHAR wsz_FROM[];					
extern WCHAR wsz_BOGUS_WHERE[];			
extern WCHAR wsz_INSERT_INTO[];			
extern WCHAR wsz_VALUES_CLAUSE[];		
extern WCHAR wsz_PARAM[];				
extern WCHAR wsz_PRIMARY_KEY[];

//General String Values
extern WCHAR wsz_COMMA[];				
extern WCHAR wsz_LPAREN[];				
extern WCHAR wsz_RPAREN[];				
extern WCHAR wsz_SPACE[];				
extern WCHAR wsz_PERIOD[];				

#define EOL			L'\0'
#define SPACE		L' '
#define UNDERSCORE	L'_'


#endif //_COMMON_H_
