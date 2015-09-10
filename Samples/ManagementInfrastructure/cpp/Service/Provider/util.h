//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#pragma once
#ifndef _UTIL_H
#define _UTIL_H
#include <windows.h>
#include <mi.h>
//
// set thread preferred UILanguages from given MI_Context and
// return original thread preferred UI Languages
// 
// Argument:
//      context     MI_Context object contains the UI Locale setting from client
//      ppOrgUILang     Orignal thread preferred UI Languages
//
void SetUILocale(
    __in MI_Context *context,
    _Deref_out_opt_z_ LPWSTR *ppOrgUILang);

//
// reset thread preferred UILanguages and release buffer
// of given parameter pOrgUILang
// 
// Argument:
//      pOrgUILang     Orignal thread preferred UI Languages
//
void ResetUILocale(__in_z LPWSTR pOrgUILang);

//
// Helper function of allocating memory from process heap
// 
// Argument:
//      dwBytes     number of bytes to allocate.
//  
// Return value:
//      allocated memory address
//
LPVOID AllocateMemory(SIZE_T dwBytes);

//
// Helper function of freeing memory
// 
// Argument:
//      lpMem       memory address to free.
//
void FreeMemory(LPVOID lpMem);

#endif
