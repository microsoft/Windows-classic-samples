/* Copyright (c) Microsoft Corporation. All rights reserved. */

#ifndef MULTISZ_H
#define MULTISZ_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Struct representing a REG_MULTI_SZ data type. 'nChar' is the length in
 * characters including the two trailing nulls. */
typedef struct tagMULTI_SZ
{
   wchar_t* pwszValue;
   DWORD nChar;
} MULTI_SZ, *PMULTI_SZ;

/* Initialize an empty MULTI_SZ. */
VOID
WINAPI
MultiSzInit(
    PMULTI_SZ pMultiSz
    );

/* Free any memory associated with a MULTI_SZ. */
VOID
WINAPI
MultiSzFree(
    PMULTI_SZ pMultiSz
    );

/* Append a string to a MULTI_SZ. */
LONG
WINAPI
MultiSzAppend(
    PMULTI_SZ pMultiSz,
    LPCWSTR pwszString
    );

/* Erase a string from within a MULTI_SZ. */
VOID
WINAPI
MultiSzErase(
    PMULTI_SZ pMultiSz,
    LPWSTR pwszString
    );

/* Find a string within a MULTI_SZ. Returns NULL if the string isn't found. */
LPWSTR
WINAPI
MultiSzFind(
    PMULTI_SZ pMultiSz,
    LPCWSTR pwszString
    );

/* Read a MULTI_SZ from the registry. */
LONG
WINAPI
MultiSzQuery(
    PMULTI_SZ pMultiSz,
    HKEY hKey,
    LPCWSTR pwszValueName
    );

/* Write a MULTI_SZ to the registry. */
LONG
WINAPI
MultiSzSet(
    PMULTI_SZ pMultiSz,
    HKEY hKey,
    LPCWSTR pwszValueName
    );

#ifdef __cplusplus
}
#endif
#endif /* MULTISZ_H */
