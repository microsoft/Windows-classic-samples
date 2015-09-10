//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#include "util.h"

//
// Helper function of allocating memory from process heap
// 
// Argument:
//      dwBytes     number of bytes to allocate.
//  
// Return value:
//      allocated memory address
//
LPVOID AllocateMemory(SIZE_T dwBytes)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytes);
}

//
// Helper function of freeing memory
// 
// Argument:
//      lpMem       memory address to free.
//
void FreeMemory(LPVOID lpMem)
{
    HeapFree(GetProcessHeap(), 0, lpMem);
}

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
    _Deref_out_opt_z_ LPWSTR *ppOrgUILang)
{
    MI_Char UILocale[MI_MAX_LOCALE_SIZE];
    MI_Result r;
    LPWSTR pOrgUILang = NULL;
    ULONG numLangs = 0;
    ULONG lenBuffer = 0;
    *ppOrgUILang = NULL;
    memset(UILocale, 0, sizeof(UILocale));
    r = MI_GetLocale(context, MI_LOCALE_TYPE_REQUESTED_UI, UILocale);
    if (r != MI_RESULT_OK)
    {
        return;
    }
    if (!GetThreadPreferredUILanguages(0, &numLangs, NULL, &lenBuffer))
    {
        return;
    }
    pOrgUILang = (LPWSTR)AllocateMemory(sizeof(wchar_t)*lenBuffer);
    if (pOrgUILang == NULL)
    {
        return;
    }
    if (!GetThreadPreferredUILanguages(0, &numLangs, pOrgUILang, &lenBuffer))
    {
        // release the buffer
        FreeMemory(pOrgUILang);
        return;
    }
    // Set the prefferred UI locale and ignore the result
    SetThreadPreferredUILanguages(0, UILocale, &numLangs);
    *ppOrgUILang = pOrgUILang;
}

//
// reset thread preferred UILanguages and release buffer
// of given parameter pOrgUILang
// 
// Argument:
//      pOrgUILang     Orignal thread preferred UI Languages
//
void ResetUILocale(__in_z LPWSTR pOrgUILang)
{
    if (NULL != pOrgUILang)
    {
        ULONG numLangs = 0;
        SetThreadPreferredUILanguages(0, pOrgUILang, &numLangs);
        FreeMemory(pOrgUILang);
    }
}
