#ifndef __COM_FAXJOB_SAMPLE
//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------


#define __COM_FAXJOB_SAMPLE

//
//Includes
//
#include <faxcomex.h>
#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <objbase.h>
#include <tchar.h>
#include <assert.h>
#include <shellapi.h>
#include <strsafe.h>

#define VISTA 6


#define INIT_VARIANT_ARRAY(_variantsArray_,_numVariants_) \
{ \
        for (DWORD _dw_ = 0; _dw_ < (_numVariants_); ++_dw_) \
        { \
                VariantInit(&((_variantsArray_)[_dw_])); \
        } \
}

#define CLEAR_VARIANT_ARRAY(_variantsArray_,_numVariants_) \
{ \
        for (DWORD _dw_ = 0; _dw_ < (_numVariants_); ++_dw_) \
        { \
                /* ignore the failure of VariantClear - can't do much anyways */ \
                VariantClear(&((_variantsArray_)[_dw_])); \
        } \
}

#endif

