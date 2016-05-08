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
//      Main entry point for WCS color sample application 
//
//----------------------------------------------------------------------------

#include "precomp.h"
#include "ColorSamples.h"

//+---------------------------------------------------------------------------
//
//  Member:
//      wmain
//
//  Synopsis:
//      Main entry point of the executable
//
//----------------------------------------------------------------------------
int __cdecl wmain(
    __in int argc, 
    // Argument count
    __in_ecount(argc) WCHAR *argv[]
    // Argugment list
    )
{
    HRESULT hr = S_OK;

    //
    // Run demonstrations of creating WCS transforms for different profile
    // types and rendering intents
    //
    CTransformCreationDemo *pTransformDemo = new CTransformCreationDemo();
    if (!pTransformDemo)
    {
        hr = E_OUTOFMEMORY;
    }
    if (SUCCEEDED(hr))
    {
        hr = pTransformDemo->RunDemos();
        if (FAILED(hr))
        {
            wprintf(L"Running Transform Demos failed. Error code 0x%X\n", hr);
        }
    }

    delete pTransformDemo;

    if (SUCCEEDED(hr))
    {
        //
        // Run demonstrations of color and bitmap translation using different
        // formats and APIs
        //
        CTranslationDemo *pTranslationDemo = new CTranslationDemo();
        if (!pTranslationDemo)
        {
            hr = E_OUTOFMEMORY;
        }
        if (SUCCEEDED(hr))
        {
            hr = pTranslationDemo->RunDemos();
            if (FAILED(hr))
            {
                wprintf(L"Running Translation Demos failed. Error code 0x%X\n", hr);
            }
        }
        delete pTranslationDemo;
    }



    return (SUCCEEDED(hr) ? 0 : -1);
}
