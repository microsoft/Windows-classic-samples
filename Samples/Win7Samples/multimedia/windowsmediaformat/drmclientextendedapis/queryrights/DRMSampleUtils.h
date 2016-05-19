///////////////////////////////////////////////////////////////////////////////
//
// DRMSampleUtils.h : Contains common macros and functions for DRM samples.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>

///////////////////////////////////////////////////////////////////////////////
// Macros
///////////////////////////////////////////////////////////////////////////////
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) \
   if(x != NULL)        \
   {                    \
      x->Release();     \
      x = NULL;         \
   }
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
   if(x != NULL)             \
   {                         \
      delete[] x;            \
      x = NULL;              \
   }
#endif

#ifndef SAFE_FILE_CLOSE
#define SAFE_FILE_CLOSE(x) \
    if(x != NULL)          \
    {                      \
        fclose(x);         \
        x = NULL;          \
    }
#endif

///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////
const wchar_t g_wszKIDFileHeaderString[] = L"KIDFILE";
const int     g_TempStringSize           = 80;


///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////
void DisplayError(HRESULT ErrorCode, const wchar_t* pwszMessage);
HRESULT ParseKIDFile(WCHAR* pwszInFile, 
                     WCHAR*** pppKIDStrings, 
                     int* pNumStrings);

