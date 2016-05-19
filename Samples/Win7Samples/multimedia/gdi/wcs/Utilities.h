//+--------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//      Contains declarations of functions not specific to the color sample
//
//----------------------------------------------------------------------------
#pragma once

#include "precomp.h"

//+---------------------------------------------------------------------------
//
//  Function:
//      GetHRESULTFromLastError
//
//  Synopsis:
//      This method returns an HRESULT based on the result of GetLastError().
//      If GetLastError does not return an error, E_FAIL will be returned.
//      This will prevent problems if a method fails, but does not call
//      SetLastError.
//
//----------------------------------------------------------------------------
inline HRESULT HRESULTFromLastError()
{
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:
//      AlignToDWORD
//
//  Synopsis:
//      This method makes the input size to be DWORD-aligned.  This increases
//      the value by up 3 in order to make it divisible by 4 (size of DWORD).
//      The result may be zero if aligning to DWORD boundary causes the value
//      to overflow.
//
//----------------------------------------------------------------------------
inline DWORD AlignToDWORD(DWORD dwInput)
{
    return (dwInput+3)&~3;
}


HRESULT SafeULongMult(
    __in ULONG ulMultiplicand,
    __in ULONG ulMultiplier,
    __out ULONG *pulResult
    );
