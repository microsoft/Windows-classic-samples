//----------------------------------------------------------------------
//
//  Microsoft Active Directory 1.0 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996
//
//  File:       main.hxx
//
//  Contents:   Main include file for adscmd
//
//
//----------------------------------------------------------------------

//
// ********* System Includes
//

#define UNICODE
#define _UNICODE
#define INC_OLE2

#include <windows.h>

//
// ********* CRunTime Includes
//

#include <stdlib.h>
#include <limits.h>
#include <io.h>
#include <stdio.h>

//
// *********  Public ADs includes
//

#include <activeds.h>

//
// *********  Useful macros
//

#define BAIL_ON_NULL(p)       \
     if (!(p)) {           \
         goto error;   \
     }

#define BAIL_ON_FAILURE(hr,msg)   \
     if (FAILED(hr)) {     \
         wprintf(L"Error: 0x%x\t Msg: %s \n", hr, msg); \
         goto error;   \
     }

#define FREE_INTERFACE(pInterface) \
     if (pInterface) {          \
         pInterface->Release(); \
         pInterface=NULL;       \
     }

#define FREE_BSTR(bstr)            \
     if (bstr) {                \
         SysFreeString(bstr);   \
         bstr = NULL;           \
     }

//
// *********  Prototypes
//

void
PrintUsage(
 void
 );

int
AnsiToUnicodeString(
 LPSTR pAnsi,
 LPWSTR pUnicode,
 DWORD StringLength
 );

int
UnicodeToAnsiString(
 LPWSTR pUnicode,
 LPSTR pAnsi,
 DWORD StringLength
 );

LPWSTR
AllocateUnicodeString(
 LPSTR  pAnsiString
 );

void
FreeUnicodeString(
 LPWSTR  pUnicodeString
 );

HRESULT
PrintVariantArray(
 VARIANT var
 );

HRESULT
PrintVariant(
 VARIANT varPropData
 );

HRESULT
PrintProperty(
 BSTR bstrPropName,
 HRESULT hRetVal,
 VARIANT varPropData
 );

HRESULT
GetPropertyList(
 IADs * pADs,
 VARIANT * pvar
 );

//
// Functions to dump contents of an object
//

int
DoDump(
 char *AnsiADsPath
 ) ;

HRESULT
DumpObject(
 IADs * pADs
 );

//
// Functions to list objects within a container.
//

int
DoList(
 char *AnsiADsPath
 ) ;

HRESULT
EnumObject(
 LPWSTR pszADsPath,
 LPWSTR * lppClassNames,
 DWORD dwClassNames
 ) ;

