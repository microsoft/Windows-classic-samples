//-----------------------------------------------------------------------------
// Microsoft OLE DB Test Table dump
// Copyright 1995-1999 Microsoft Corporation.  
//
// @doc
//
// @module COMMON.H
//
//-----------------------------------------------------------------------------------

#ifndef _COMMON_H_
#define _COMMON_H_

//suppress warnings about calling "unsecure" string functions
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif	//_CRT_SECURE_NO_WARNINGS

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif	//_CRT_SECURE_NO_DEPRECATE

//suppress warnings about calling unconformanced swprintf
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif	//_CRT_NON_CONFORMING_SWPRINTFS

///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "oledb.h"
#include "oledberr.h"

#include "Privlib.h"	//Private Library

#include <locale.h>		//setlocale
#include <time.h>		//timestamps in file...

#include "Error.h"
#include "resource.h"


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
#define XTEST(hr)						{ DisplayAllErrors(NULL, hr, __FILE__, __LINE__); }
#define XTESTC(hr)						{ if(FAILED(DisplayAllErrors(NULL, hr, __FILE__, __LINE__))) goto CLEANUP; }

#undef  TESTC
#define TESTC(hr)						{ if(FAILED(hr)) { goto CLEANUP;  } }


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
#define MAX_QUERY_LEN			4096
#define MAX_NAME_LEN			256
#define MAX_BUFFER_SIZE		    5000

#define EOL		'\0'
#define wEOL	L'\0'


////////////////////////////////////////////////////////////////////////////
// OLEDB General Helper functions
//
////////////////////////////////////////////////////////////////////////////
void	DumpProviderInfo(WCHAR* pwszProvider, WCHAR* pwszTableName, CHAR* pszCmdLine);
void	DumpURLInfo(WCHAR* pwszDefURL, WCHAR* pwszRowURL, WCHAR* pwszTableName);
HRESULT	DumpQueryInfo(CTable* pCTable, WCHAR* pwszDefaultQuery, WCHAR* pwszTableName);
void	DumpColumnsInfo(DBORDINAL cColumns, DBCOLUMNINFO* rgColInfo,	CTable* pCTable );
void	DumpRow(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData);
void	DumpColumn(DBBINDING* pBinding, void* pData);

void	OutputFile(const CHAR* format, ...);
void	OutputText(const CHAR* format, ...);
void	OutputText(const WCHAR* format, ...);
void	DumpLineEnd();

void	ParseCmdLine(short* argc, WCHAR* argv[], ULONG* pcPropSets, DBPROPSET** prgPropSets);

CHAR*	GetNoteStringBitvals(ULONG cNameMap, const NAMEMAP* rgNameMap, DWORD dwValue);


#endif //_COMMON_H_
