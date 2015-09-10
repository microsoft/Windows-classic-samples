// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx Bundle APIs to produce
// an Appx Bundle from a list of app packages and resource packages on disk.
//
// For simplicity, the list of packages to be bundled is hard coded in this
// sample.  A fully functional application might get its input from other
// sources.

#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>

#include <AppxPackaging.h>  // For Appx Bundle APIs

#include "CreateBundle.h"

// Path where all input files are stored
const LPCWSTR DataPath = L"Data\\";

// The produced bundle's content consists of these files taken from the data folder
const int PayloadPackagesCount = 2;
const LPCWSTR PayloadPackagesName[PayloadPackagesCount] = {
    L"MainAppPackage.appx",
    L"ResourcePackage.lang-de.appx"
};

// The produced package will be stored under this file name
const LPCWSTR OutputBundlePath = L"sample.appxbundle";

//
// Function to create a readable IStream over the file whose name is the
// concatenation of the path and fileName parameters.  For simplicity, file
// names including path are assumed to be 100 characters or less.  A real
// application should be able to handle longer names and allocate the
// necessary buffer dynamically.
//
// Parameters:
// path - Path of the folder containing the file to be opened, ending with a
//        slash ('\') character
// fileName - Name, not including path, of the file to be opened
// stream - Output parameter pointing to the created instance of IStream over
//          the specified file when this function succeeds.
//
HRESULT GetFileStream(
    _In_ LPCWSTR path,
    _In_ LPCWSTR fileName,
    _Outptr_ IStream** stream)
{
    HRESULT hr = S_OK;
    const int MaxFileNameLength = 100;
    WCHAR fullFileName[MaxFileNameLength + 1];

    // Create full file name by concatenating path and fileName
    hr = StringCchCopyW(fullFileName, MaxFileNameLength, path);
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(fullFileName, MaxFileNameLength, fileName);
    }

    // Create stream for reading the file
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFileEx(
                fullFileName,
                STGM_READ | STGM_SHARE_EXCLUSIVE,
                0, // default file attributes
                FALSE, // do not create new file
                NULL, // no template
                stream);
    }
    return hr;
}

//
// Function to create an Appx Bundle writer with default settings, given the
// output file name.
//
// Parameters:
// outputFileName - Name including path to the bundle (.appxbundle file) to
//                  be created.
// writer - Output parameter pointing to the created instance of
//          IAppxBundleWriter when this function succeeds.
//
HRESULT GetBundleWriter(
    _In_ LPCWSTR outputFileName,
    _Outptr_ IAppxBundleWriter** writer)
{
    HRESULT hr = S_OK;
    IStream* outputStream = NULL;
    IAppxBundleFactory* appxBundleFactory = NULL;

    // Create a stream over the output file where the bundle will be written
    hr = SHCreateStreamOnFileEx(
            outputFileName,
            STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
            0, // default file attributes
            TRUE, // create file if it does not exist
            NULL, // no template
            &outputStream);

    // Create a new Appx Bundle factory
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(
                __uuidof(AppxBundleFactory),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IAppxBundleFactory),
                (LPVOID*)(&appxBundleFactory));
    }

    // Create a new bundle writer using the factory
    if (SUCCEEDED(hr))
    {
        hr = appxBundleFactory->CreateBundleWriter(
                outputStream,
                0, // by specifying 0, the bundle will have an automatically
                   // generated version number based on the current time
                writer);
    }

    // Clean up allocated resources
    if (appxBundleFactory != NULL)
    {
        appxBundleFactory->Release();
        appxBundleFactory = NULL;
    }
    if (outputStream != NULL)
    {
        outputStream->Release();
        outputStream = NULL;
    }
    return hr;
}

//
// Main entry point of the sample
//
int wmain()
{
    wprintf(L"Copyright (c) Microsoft Corporation.  All rights reserved.\n");
    wprintf(L"CreateBundle sample\n\n");

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Create a bundle writer object
        IAppxBundleWriter* bundleWriter = NULL;

        wprintf(L"\nCreating bundle writer\n\n");

        hr = GetBundleWriter(OutputBundlePath, &bundleWriter);

        // Add all payload packages to the package writer
        for (int i = 0; SUCCEEDED(hr) && (i < PayloadPackagesCount); i++)
        {
            IStream* payloadPackageStream = NULL;

            wprintf(L"Adding payload package: %s\n", PayloadPackagesName[i]);

            hr = GetFileStream(DataPath, PayloadPackagesName[i], &payloadPackageStream);

            if (SUCCEEDED(hr))
            {
                bundleWriter->AddPayloadPackage(
                    PayloadPackagesName[i],
                    payloadPackageStream);
            }

            if (payloadPackageStream != NULL)
            {
                payloadPackageStream->Release();
                payloadPackageStream = NULL;
            }
        }

        // Close the bundle writer to flush the output stream
        if (SUCCEEDED(hr))
        {
            hr = bundleWriter->Close();
        }

        // Clean up allocated resources
        if (bundleWriter != NULL)
        {
            bundleWriter->Release();
            bundleWriter = NULL;
        }
        CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nBundle successfully saved to %s.\n", OutputBundlePath);
    }
    else
    {
        wprintf(L"\nBundle creation failed with HRESULT 0x%08X.\n", hr);
    }
    return SUCCEEDED(hr) ? 0 : 1;
}
