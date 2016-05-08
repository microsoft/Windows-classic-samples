//+-------------------------------------------------------------------------
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       common.h
//
//--------------------------------------------------------------------------

#ifndef __COMMON_H_5F90F583_B9A4_4A8F_91BC_618DE6696231_
#define __COMMON_H_5F90F583_B9A4_4A8F_91BC_618DE6696231_

#include <windows.h>

/*--------------------------------------------------------------------------
 *
 * Predefined Resource Types
 *
 --------------------------------------------------------------------------*/
#define RT_INSTALL_PROPERTY  MAKEINTRESOURCE(40)

/*--------------------------------------------------------------------------
 *
 * Predefined Resource Names
 *
 --------------------------------------------------------------------------*/
#define ISETUPPROPNAME_BASEURL              TEXT("BASEURL")
#define ISETUPPROPNAME_DATABASE             TEXT("DATABASE")
#define ISETUPPROPNAME_OPERATION            TEXT("OPERATION")
#define ISETUPPROPNAME_MINIMUM_MSI          TEXT("MINIMUM_MSI")
#define ISETUPPROPNAME_UPDATELOCATION       TEXT("UPDATELOCATION")
#define ISETUPPROPNAME_UPDATE               TEXT("UPDATE")
#define ISETUPPROPNAME_PRODUCTNAME          TEXT("PRODUCTNAME")
#define ISETUPPROPNAME_PROPERTIES           TEXT("PROPERTIES")
#define ISETUPPROPNAME_PATCH                TEXT("PATCH")

/*--------------------------------------------------------------------------
 *
 * Common Prototypes
 *
 ---------------------------------------------------------------------------*/
UINT LoadResourceString(HINSTANCE hInst, LPCSTR lpType, LPCSTR lpName, __out_ecount(*pdwBufSize) LPSTR lpBuf, DWORD *pdwBufSize);
UINT SetupLoadResourceString(HINSTANCE hInst, LPCSTR lpName, __deref_out LPSTR *lppBuf, DWORD dwBufSize);

#endif //__COMMON_H_5F90F583_B9A4_4A8F_91BC_618DE6696231_
