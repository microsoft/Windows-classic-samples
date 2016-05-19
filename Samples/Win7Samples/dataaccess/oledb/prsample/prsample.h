//---------------------------------------------------------------------------
// Microsoft OLE DB Programmer's Reference Sample
// Copyright (C) 1998 By Microsoft Corporation.
//	  
// @doc
//												  
// @module PRSAMPLE.H
//
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////
// Includes					 
//							
/////////////////////////////////////////////////////////////////
#ifndef __PRSAMPLE_H__
#define	__PRSAMPLE_H__

#include "oledb.h"			// OLE DB Header
#include "oledberr.h"		// OLE DB Errors

#include "msdasc.h"			// OLE DB Service Component header
#include "msdaguid.h"		// OLE DB Root Enumerator 
#include "msdasql.h"		// MSDASQL - Default provider

#include <stdio.h>			// input and output functions
#include <conio.h>			// getch, putch
#include <locale.h>			// setlocale

/////////////////////////////////////////////////////////////////
// Globals					 
//
/////////////////////////////////////////////////////////////////
extern DWORD g_dwFlags;


/////////////////////////////////////////////////////////////////
// Defines					 							   
//
/////////////////////////////////////////////////////////////////
#define __LONGSTRING(string) L##string
#define LONGSTRING(string) __LONGSTRING(string)

//Goes to CLEANUP on Failure
#define CHECK_HR(hr)			\
	if(FAILED(hr))				\
		goto CLEANUP

//Goes to CLEANUP on Failure, and displays any ErrorInfo
#define XCHECK_HR(hr)												\
{																	\
	if( g_dwFlags & DISPLAY_METHODCALLS )							\
		fwprintf(stderr, LONGSTRING(#hr) L"\n");					\
	if(FAILED(myHandleResult(hr, LONGSTRING(__FILE__), __LINE__)))	\
		goto CLEANUP;												\
}
#define CHECK_MEMORY(hr, pv)	\
{								\
	if(!pv) 					\
	{							\
		hr = E_OUTOFMEMORY;		\
		CHECK_HR(hr);			\
	}							\
}


#define MAX_COL_SIZE		    5000
#define MAX_NAME_LEN		    256

#define MAX_ROWS				10
#define MAX_DISPLAY_SIZE		20
#define MIN_DISPLAY_SIZE		 3

//ROUNDUP on all platforms pointers must be aligned properly
#define ROUNDUP_AMOUNT	8
#define ROUNDUP_(size,amount)	(((DBBYTEOFFSET)(size)+((amount)-1))&~((DBBYTEOFFSET)(amount)-1))
#define ROUNDUP(size)			ROUNDUP_(size, ROUNDUP_AMOUNT)

enum
{
	//Connecting
	USE_PROMPTDATASOURCE		= 0x0001,
	USE_ENUMERATOR				= 0x0002,

	//Rowset
	USE_COMMAND					= 0x0010,

	//Storage Objects
	USE_ISEQSTREAM				= 0x0100,

	// Display options
	DISPLAY_METHODCALLS			= 0x1000,
	DISPLAY_INSTRUCTIONS		= 0x2000,
};


/////////////////////////////////////////////////////////////////////////////
// Function Prototypes
//
/////////////////////////////////////////////////////////////////////////////
//Main
BOOL	myParseCommandLine();
void	myDisplayInstructions();
BOOL	myGetInputFromUser(LPWSTR pwszInput, size_t nInputBufferSize, LPCWSTR pwszFmt, ...);
CHAR	myGetChar();

//Enumerator
HRESULT myCreateEnumerator(REFCLSID clsidEnumerator, CLSID* pCLSID);

//DataSource
HRESULT myCreateDataSource(IUnknown** ppUnkDataSource);
HRESULT myDoInitialization(IUnknown* pIUnknown);
HRESULT myGetProperty(IUnknown* pIUnknown, REFIID riid, DBPROPID dwPropertyID, REFGUID guidPropertySet, BOOL* pbValue);
void	myAddProperty(DBPROP* pProp, DBPROPID dwPropertyID, VARTYPE vtType = VT_BOOL, LONG_PTR lValue = VARIANT_TRUE, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_OPTIONAL);

//Session
HRESULT	myCreateSession(IUnknown* pUnkDataSource, IUnknown** ppUnkSession);
HRESULT	myCreateSchemaRowset(GUID guidSchema, IUnknown* pUnkSession, ULONG cchBuffer, LPWSTR pwszBuffer);

//Command
HRESULT	myCreateCommand(IUnknown* pUnkSession, IUnknown** ppUnkCommand);
HRESULT	myExecuteCommand(IUnknown* pUnkCommand, WCHAR* pwszCommandText, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppUnkRowset);

//Rowset
HRESULT	myCreateRowset(IUnknown* pUnkSession, IUnknown** ppUnkRowset);
HRESULT mySetupBindings(IUnknown* pUnkRowset, DBORDINAL* pcBindings, DBBINDING** prgBindings, DBORDINAL* pcbRowSize);
HRESULT myCreateAccessor(IUnknown* pUnkRowset, HACCESSOR* phAccessor, DBORDINAL* pcBindings, DBBINDING** prgBindings, DBORDINAL* pcbRowSize);

HRESULT myDisplayRowset(IUnknown* pUnkRowset, LPCWSTR pwszColToReturn, ULONG cchBuffer, LPWSTR pwszBuffer);
HRESULT myDisplayColumnNames(IUnknown* pUnkRowset, ULONG* rgDispSize);
HRESULT myDisplayRow(ULONG iRow, DBORDINAL cBindings, DBBINDING* rgBindings, void* pData, ULONG * rgDispSize);

HRESULT myInteractWithRowset(IRowset* pIRowset, LONG* pcRows, DBCOUNTITEM cRowsObtained, BOOL fCanFetchBackwards, void* pData, DBORDINAL cbRowSize, DBBINDING* pBinding, ULONG cchBuffer, LPWSTR pwszBuffer);
HRESULT myFindColumn(IUnknown * pUnkRowset, LPCWSTR pwszName, LONG* plIndex);
HRESULT myUpdateDisplaySize(DBORDINAL cBindings, DBBINDING* rgBindings, void* pData, ULONG* rgDispSize);
void	myFreeBindings(DBORDINAL cBindings, DBBINDING* rgBindings);
void	myAddRowsetProperties(DBPROPSET* pPropSet, ULONG cProperties, DBPROP* rgProperties);

//Error
HRESULT myHandleResult(HRESULT hrReturned, LPCWSTR pwszFile, ULONG ulLine);
HRESULT myDisplayErrorRecord(HRESULT hrReturned, ULONG iRecord, IErrorRecords* pIErrorRecords, LPCWSTR pwszFile, ULONG ulLine);
HRESULT myDisplayErrorInfo(HRESULT hrReturned, IErrorInfo* pIErrorInfo, LPCWSTR pwszFile, ULONG ulLine);
HRESULT myGetSqlErrorInfo(ULONG iRecord, IErrorRecords* pIErrorRecords, BSTR* pBstr, LONG* plNativeError);


#endif	//__PRSAMPLE_H__
