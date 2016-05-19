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
//      Contains definitions of  functions not specific to the color sample
//
//----------------------------------------------------------------------------

#include "precomp.h"
#include "Utilities.h"

//+---------------------------------------------------------------------------
//
//  Function:
//      SafeULongMult
//
//  Synopsis:
//      This function multiplies two ULONG numbers together,
//      and returns E_FAIL if numeric overflow occurs.
//
//----------------------------------------------------------------------------
HRESULT SafeULongMult(
    __in ULONG ulMultiplicand,
    __in ULONG ulMultiplier,
    __out ULONG *pulResult
    )
{
    ULONG64 ul64Multiplicand = ulMultiplicand;
    ULONG64 ul64Multiplier = ulMultiplier;
    ULONG64 Product = ul64Multiplicand*ul64Multiplier;
    if (Product <= ((ULONG)-1))
    {
        *pulResult = (ULONG)Product;
        return S_OK;
    }
    return E_FAIL;
}