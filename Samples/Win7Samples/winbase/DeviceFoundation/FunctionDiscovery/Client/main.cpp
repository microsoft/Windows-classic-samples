// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// The class that wraps Function Discovery in this sample
#include "FunDiscovery.h"
#include "strsafe.h"
#include "conio.h"
#include <objbase.h>

int _cdecl wmain()
{
    wprintf(L"This sample will list all PnP devices on your system.\n");
    wprintf(L"Then, it will demonstrate Function Discovery's ADD and\n");
    wprintf(L"REMOVE notifications.\nPress any key to continue.\n");
    _getwch( );

    HRESULT hr = S_OK;

    hr = ::CoInitializeEx( NULL, COINIT_MULTITHREADED );

    // Instantiate the Function Discovery wrapper object
    CMyFDHelper * pFD = new CMyFDHelper( );
    if( NULL == pFD )
        hr = E_OUTOFMEMORY;

    if( S_OK == hr)
        hr = pFD->Initialize();

    // Enumerate all PnP Devices
    if( S_OK == hr )
        hr = pFD->ListFunctionInstances( FCTN_CATEGORY_PNP );

    // Wait for device to be added
    wprintf(L"Waiting 30 seconds for you to plug in a PnP device.\n");
    wprintf(L"For example, a USB mouse.\n");
    if( S_OK == hr )
        hr = pFD->WaitForChange( 30000,
        FCTN_CATEGORY_PNP,
        QUA_ADD );

    // Wait for device to be removed
    if( S_OK == hr )
    {
        wprintf(L"Waiting 30 seconds for you to remove a device...\n");

        hr = pFD->WaitForChange( 30000,
            FCTN_CATEGORY_PNP,
            QUA_REMOVE );
    }

    else if( RPC_S_CALLPENDING == hr )
    {
        wprintf(L"No device was added.  ");
        wprintf(L"Try running the sample again.");
    }

    if( NULL != pFD )
        pFD->Release( );

    ::CoUninitialize();

    return hr;
}    
