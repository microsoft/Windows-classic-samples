// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx packaging APIs to produce
// an Appx package from a list of files on disk.
//
// For the sake of simplicity, the list of files to be packaged is hard coded
// in this sample.  A fully functional application might read its list of
// files from user input, or even generate content dynamically.

#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>

#include <AppxPackaging.h>  // For Appx Packaging APIs

#include "CreateAppx.h"

// Path where all input files are stored
const LPCWSTR DataPath = L"Data\\";

// The produced Appx package's content consists of these files, with
// corresponding content types and compression options.
const int PayloadFilesCount = 4;
const LPCWSTR PayloadFilesName[PayloadFilesCount] = {
    L"AppTile.png",
    L"Default.html",
    L"images\\smiley.jpg",
    L"Error.html",
};
const LPCWSTR PayloadFilesContentType[PayloadFilesCount] = {
    L"image/png",
    L"text/html",
    L"image/jpeg",
    L"text/html",
};
const APPX_COMPRESSION_OPTION PayloadFilesCompression[PayloadFilesCount] = {
    APPX_COMPRESSION_OPTION_NONE,
    APPX_COMPRESSION_OPTION_NORMAL,
    APPX_COMPRESSION_OPTION_NONE,
    APPX_COMPRESSION_OPTION_NORMAL,
};

// The Appx package's manifest is read from this file
const LPCWSTR ManifestFileName = L"AppxManifest.xml";

// The hash algorithm to be used for the package's block map is SHA2-256
const LPCWSTR Sha256AlgorithmUri = L"http://www.w3.org/2001/04/xmlenc#sha256";

// The produced package will be stored under this file name
const LPCWSTR OutputPackagePath = L"HelloWorld.appx";

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
// Function to create an Appx package writer with default settings, given the
// output file name.
//
// Parameters:
// outputFileName - Name including path to the Appx package (.appx file) to be
//                  created.
// writer - Output parameter pointing to the created instance of
//          IAppxPackageWriter when this function succeeds.
//
HRESULT GetPackageWriter(
    _In_ LPCWSTR outputFileName,
    _Outptr_ IAppxPackageWriter** writer)
{
    HRESULT hr = S_OK;
    IStream* outputStream = NULL;
    IUri* hashMethod = NULL;
    APPX_PACKAGE_SETTINGS packageSettings = {0};
    IAppxFactory* appxFactory = NULL;

    // Create a stream over the output file where the package will be written
    hr = SHCreateStreamOnFileEx(
            outputFileName,
            STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
            0, // default file attributes
            TRUE, // create file if it does not exist
            NULL, // no template
            &outputStream);

    // Create default package writer settings, including hash algorithm URI
    // and Zip format.
    if (SUCCEEDED(hr))
    {
        hr = CreateUri(
                Sha256AlgorithmUri,
                Uri_CREATE_CANONICALIZE,
                0, // reserved parameter
                &hashMethod);
    }

    if (SUCCEEDED(hr))
    {
        packageSettings.forceZip32 = TRUE;
        packageSettings.hashMethod = hashMethod;
    }

    // Create a new Appx factory
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(
                __uuidof(AppxFactory),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IAppxFactory),
                (LPVOID*)(&appxFactory));
    }

    // Create a new package writer using the factory
    if (SUCCEEDED(hr))
    {
        hr = appxFactory->CreatePackageWriter(
                outputStream,
                &packageSettings,
                writer);
    }

    // Clean up allocated resources
    if (appxFactory != NULL)
    {
        appxFactory->Release();
        appxFactory = NULL;
    }
    if (hashMethod != NULL)
    {
        hashMethod->Release();
        hashMethod = NULL;
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
    wprintf(L"CreateAppx sample\n\n");

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Create a package writer
        IAppxPackageWriter* packageWriter = NULL;
        IStream* manifestStream = NULL;

        wprintf(L"\nCreating package writer\n\n");

        hr = GetPackageWriter(OutputPackagePath, &packageWriter);

        // Add all payload files to the package writer
        for (int i = 0; SUCCEEDED(hr) && (i < PayloadFilesCount); i++)
        {
            IStream* fileStream = NULL;

            wprintf(L"Adding file: %s\n", PayloadFilesName[i]);

            hr = GetFileStream(DataPath, PayloadFilesName[i], &fileStream);

            if (SUCCEEDED(hr))
            {
                packageWriter->AddPayloadFile(
                    PayloadFilesName[i],
                    PayloadFilesContentType[i],
                    PayloadFilesCompression[i],
                    fileStream);
            }

            if (fileStream != NULL)
            {
                fileStream->Release();
                fileStream = NULL;
            }
        }

        // Add manifest to package and close package writer
        if (SUCCEEDED(hr))
        {
            wprintf(L"\nClosing package writer and adding AppxManifest.xml as the package manifest\n");
            hr = GetFileStream(DataPath, ManifestFileName, &manifestStream);
        }
        if (SUCCEEDED(hr))
        {
            hr = packageWriter->Close(manifestStream);
        }

        // Clean up allocated resources
        if (manifestStream != NULL)
        {
            manifestStream->Release();
            manifestStream = NULL;
        }
        if (packageWriter != NULL)
        {
            packageWriter->Release();
            packageWriter = NULL;
        }
        CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nPackage successfully saved to %s.\n", OutputPackagePath);
    }
    else
    {
        wprintf(L"\nPackage creation failed with HRESULT 0x%08X.\n", hr);
    }
    return SUCCEEDED(hr) ? 0 : 1;
}
