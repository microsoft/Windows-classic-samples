//////////////////////////////////////////////////////////////////////////
//
// main.cpp - Defines the entry point for the console application.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// This sample demonstrates how to perform simple transcoding
// to WMA or WMV.
//
////////////////////////////////////////////////////////////////////////// 

#include "Transcode.h"

int wmain(int argc, wchar_t* argv[])
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (argc != 3)
    {
        wprintf_s(L"Usage: %s input_file output_file\n", argv[0]);
        return 0;
    }

    const WCHAR* sInputFile = argv[1];  // Audio source file name
    const WCHAR* sOutputFile = argv[2];  // Output file name
    
    HRESULT hr = S_OK;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        hr = MFStartup(MF_VERSION);
    }

    if (SUCCEEDED(hr))
    {
        CTranscoder transcoder;

        // Create a media source for the input file.
        hr = transcoder.OpenFile(sInputFile);

        if (SUCCEEDED(hr))
        {
            wprintf_s(L"Opened file: %s.\n", sInputFile);

            //Configure the profile and build a topology.
            hr = transcoder.ConfigureAudioOutput();
        }

        if (SUCCEEDED(hr))
        {
            hr = transcoder.ConfigureVideoOutput();
        }
    
        if (SUCCEEDED(hr))
        {
            hr = transcoder.ConfigureContainer();
        }

        //Transcode and generate the output file.

        if (SUCCEEDED(hr))
        {
            hr = transcoder.EncodeToFile(sOutputFile);
        }
    
        if (SUCCEEDED(hr))
        {
            wprintf_s(L"Output file created: %s\n", sOutputFile);
        }
    }

    MFShutdown();
    CoUninitialize();

    if (FAILED(hr))
    {
        wprintf_s(L"Could not create the output file (0x%X).\n", hr);
    }

    return 0;
}

