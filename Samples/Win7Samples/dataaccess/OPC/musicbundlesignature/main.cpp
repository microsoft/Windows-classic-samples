//<SnippetMusicBundleSig_cppMainWholePage>
/*****************************************************************************
*
* File: main.cpp
*
* Description:
* This sample is a simple application that might be used as a starting-point
* for an application that uses the Packaging API. This sample demonstrates
* signature generation and validation using a sample signing policy described
* in Sign.h
*
* ------------------------------------
*
*  This file is part of the Microsoft Windows SDK Code Samples.
* 
*  Copyright (C) Microsoft Corporation.  All rights reserved.
* 
* This source code is intended only as a supplement to Microsoft
* Development Tools and/or on-line documentation.  See these other
* materials for detailed information regarding Microsoft code samples.
* 
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
* 
****************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <shlobj.h>

#include <msopc.h>  // For OPC APIs

#include <WinCrypt.h>   // For certificate stuff
#include <CryptDlg.h>   // For certificate selection dialog

#include <strsafe.h>
#include <new>

#include "Util.h"
#include "Sign.h"
#include "Validate.h"


//=================================
// Main entry point of the sample.
//=================================
int
wmain(
    )
{
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // CoUninitialize is called.
        IOpcFactory * opcFactory = NULL;

        hr = CoCreateInstance(
                __uuidof(OpcFactory), 
                NULL, 
                CLSCTX_INPROC_SERVER,
                __uuidof(IOpcFactory), 
                (LPVOID*)&opcFactory
                );

        if (SUCCEEDED(hr))
        {
            fwprintf(stdout, L"Step 1 - Sign the music bundle.\n");

            hr = SignMusicBundle(opcFactory);
        }

        if (SUCCEEDED(hr))
        {
            fwprintf(stdout, L"Step 2 - Validate the signature of the signed music bundle.\n");

            hr = ValidateMusicBundleSignature(opcFactory);
        }

        if (opcFactory)
        {
            opcFactory->Release();
            opcFactory = NULL;
        }

        //
        // Only CoUninitialize if CoInitialize succeeded.
        //
        CoUninitialize();
    }

    if (FAILED(hr))
    {
        fwprintf(stderr, L"This sample failed with HRESULT hr=0x%x\n", hr);
    }
    else
    {
        fwprintf(stdout, L"This sample finished successfully.\n");
    }

    return SUCCEEDED(hr) ? 0 : 1;
}




//</SnippetMusicBundleSig_cppMainWholePage>