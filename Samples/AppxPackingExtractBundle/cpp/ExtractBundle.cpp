// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx Bundle APIs to read the
// contents of an Appx bundle, and extract its contents to a folder on disk.

#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>

#include <AppxPackaging.h>  // For Appx Bundle APIs

#include "ExtractBundle.h"

// Types of footprint files in a bundle
const int FootprintFilesCount = 3;
const APPX_BUNDLE_FOOTPRINT_FILE_TYPE FootprintFilesType[FootprintFilesCount] = {
    APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST,
    APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP,
    APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE
};
const LPCWSTR FootprintFilesName[FootprintFilesCount] = {
    L"manifest",
    L"block map",
    L"digital signature"
};

//
// Function to create a writable IStream over a file with the specified name
// under the specified path.  This function will also create intermediate
// subdirectories if necessary.  For simplicity, file names including path are
// assumed to be 200 characters or less.  A real application should be able to
// handle longer names and allocate the necessary buffer dynamically.
//
// Parameters:
// path - Path of the folder containing the file to be opened.  This should NOT
//        end with a slash ('\') character.
// fileName - Name, not including path, of the file to be opened
// stream - Output parameter pointing to the created instance of IStream over
//          the specified file when this function succeeds.
//
HRESULT GetOutputStream(
    _In_ LPCWSTR path,
    _In_ LPCWSTR fileName,
    _Outptr_ IStream** stream)
{
    HRESULT hr = S_OK;
    const int MaxFileNameLength = 200;
    WCHAR fullFileName[MaxFileNameLength + 1];

    // Create full file name by concatenating path and fileName
    hr = StringCchCopyW(fullFileName, MaxFileNameLength, path);

    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(fullFileName, MaxFileNameLength, L"\\");
    }
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(fullFileName, MaxFileNameLength, fileName);
    }

    // Search through fullFileName for the '\' character which denotes
    // subdirectory and create each subdirectory in order of depth.
    for (int i = 0; SUCCEEDED(hr) && (i < MaxFileNameLength); i++)
    {
        if (fullFileName[i] == L'\0')
        {
            break;
        }
        else if (fullFileName[i] == L'\\')
        {
            // Temporarily set string to terminate at the '\' character
            // to obtain name of the subdirectory to create
            fullFileName[i] = L'\0';

            if (!CreateDirectory(fullFileName, NULL))
            {
                DWORD lastError = GetLastError();

                // It is normal for CreateDirectory to fail if the subdirectory
                // already exists.  Other errors should not be ignored.
                if (lastError != ERROR_ALREADY_EXISTS)
                {
                    hr = HRESULT_FROM_WIN32(lastError);
                }
            }

            // Restore original string
            fullFileName[i] = L'\\';
        }
    }

    // Create stream for writing the file
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFileEx(
                fullFileName,
                STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
                0, // default file attributes
                TRUE, // create new file if it does not exist
                NULL, // no template
                stream);
    }
    return hr;
}

//
// Function to print some basic information about an IAppxFile object, and
// write it to disk under the given path.
//
// Parameters:
// file - Instance of IAppxFile obtained from IAppxBundleReader representing
//        a footprint file or payload package in the bundle.
// outputPath - Path of the folder where extracted files should be placed
//
HRESULT ExtractFile(
    _In_ IAppxFile* file,
    _In_ LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    LPWSTR fileName = NULL;
    LPWSTR contentType = NULL;
    UINT64 fileSize = 0;
    IStream* fileStream = NULL;
    IStream* outputStream = NULL;
    ULARGE_INTEGER fileSizeLargeInteger = {0};

    // Get basic information about the file
    hr = file->GetName(&fileName);

    if (SUCCEEDED(hr))
    {
        hr = file->GetContentType(&contentType);
    }
    if (SUCCEEDED(hr))
    {
        hr = file->GetSize(&fileSize);
        fileSizeLargeInteger.QuadPart = fileSize;
    }
    if (SUCCEEDED(hr))
    {
        wprintf(L"\nFile name: %s\n", fileName);
        wprintf(L"Content type: %s\n", contentType);
        wprintf(L"Size: %llu bytes\n", fileSize);
    }

    // Write the file out to disk
    if (SUCCEEDED(hr))
    {
        hr = file->GetStream(&fileStream);
    }
    if (SUCCEEDED(hr) && (fileName != NULL))
    {
        hr = GetOutputStream(outputPath, fileName, &outputStream);
    }
    if (SUCCEEDED(hr) && (outputStream != NULL))
    {
        hr = fileStream->CopyTo(outputStream, fileSizeLargeInteger, NULL, NULL);
    }

    // String buffers obtained from the bundle APIs must be freed
    CoTaskMemFree(fileName);
    CoTaskMemFree(contentType);

    // Clean up other allocated resources
    if (outputStream != NULL)
    {
        outputStream->Release();
        outputStream = NULL;
    }
    if (fileStream != NULL)
    {
        fileStream->Release();
        fileStream = NULL;
    }
    return hr;
}

//
// Function to extract all footprint files from a bundle reader.
//
// Parameters:
// bundleReader - Instance of IAppxBundleReader over the bundle whose footprint
//                files are to be extracted.
// outputPath - Path of the folder where all extracted footprint files should
//              be placed.
//
HRESULT ExtractFootprintFiles(
    _In_ IAppxBundleReader* bundleReader,
    _In_ LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    wprintf(L"\nExtracting footprint files from the bundle\n");

    for (int i = 0; SUCCEEDED(hr) && (i < FootprintFilesCount); i++)
    {
        IAppxFile* footprintFile = NULL;

        hr = bundleReader->GetFootprintFile(FootprintFilesType[i], &footprintFile);

        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            // Some footprint files are optional, it's normal for the GetFootprintFile
            // call to fail when the file is not present.
            wprintf(L"\nThe bundle doesn't contain a %s.\n", FootprintFilesName[i]);
            hr = S_OK;
        }
        else if (SUCCEEDED(hr))
        {
            hr = ExtractFile(footprintFile, outputPath);
        }

        if (footprintFile != NULL)
        {
            footprintFile->Release();
            footprintFile = NULL;
        }
    }
    return hr;
}

//
// Function to extract all payload packages from a bundle reader.
//
// Parameters:
// bundleReader - Instance of IAppxBundleReader over the bundle whose payload
//                packages are to be extracted.
// outputPath - Path of the folder where all extracted payload packages should
//              be placed.
//
HRESULT ExtractPayloadPackages(
    _In_ IAppxBundleReader* bundleReader,
    _In_ LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    IAppxFilesEnumerator* payloadPackages = NULL;
    wprintf(L"\nExtracting payload packages from the bundle\n");

    // Get an enumerator of all payload packages from the bundle reader and
    // iterate through all of them.
    hr = bundleReader->GetPayloadPackages(&payloadPackages);

    if (SUCCEEDED(hr))
    {
        BOOL hasCurrent = FALSE;
        hr = payloadPackages->GetHasCurrent(&hasCurrent);

        while (SUCCEEDED(hr) && hasCurrent)
        {
            IAppxFile* payloadPackage = NULL;

            hr = payloadPackages->GetCurrent(&payloadPackage);

            if (SUCCEEDED(hr))
            {
                hr = ExtractFile(payloadPackage, outputPath);
            }
            if (SUCCEEDED(hr))
            {
                hr = payloadPackages->MoveNext(&hasCurrent);
            }

            if (payloadPackage != NULL)
            {
                payloadPackage->Release();
                payloadPackage = NULL;
            }
        }
    }

    if (payloadPackages != NULL)
    {
        payloadPackages->Release();
        payloadPackages = NULL;
    }
    return hr;
}

//
// Function to create an Appx Bundle reader given the input file name.
//
// Parameters:
// inputFileName - Name including path to the bundle (.appxbundle file)
//                 to be opened.
// bundleReader - Output parameter pointing to the created instance of
//                IAppxBundleReader when this function succeeds.
//
HRESULT GetBundleReader(
    _In_ LPCWSTR inputFileName,
    _Outptr_ IAppxBundleReader** bundleReader)
{
    HRESULT hr = S_OK;
    IAppxBundleFactory* appxBundleFactory = NULL;
    IStream* inputStream = NULL;

    // Create a new Appx bundle factory
    hr = CoCreateInstance(
            __uuidof(AppxBundleFactory),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IAppxBundleFactory),
            (LPVOID*)(&appxBundleFactory));

    // Create a stream over the input Appx bundle
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFileEx(
                inputFileName,
                STGM_READ | STGM_SHARE_EXCLUSIVE,
                0, // default file attributes
                FALSE, // do not create new file
                NULL, // no template
                &inputStream);
    }

    // Create a new bundle reader using the factory
    if (SUCCEEDED(hr))
    {
        hr = appxBundleFactory->CreateBundleReader(
                inputStream,
                bundleReader);
    }

    // Clean up allocated resources
    if (inputStream != NULL)
    {
        inputStream->Release();
        inputStream = NULL;
    }
    if (appxBundleFactory != NULL)
    {
        appxBundleFactory->Release();
        appxBundleFactory = NULL;
    }
    return hr;
}

//
// Main entry point of the sample
//
int wmain(
    _In_ int argc,
    _In_reads_(argc) wchar_t** argv)
{
    wprintf(L"Copyright (c) Microsoft Corporation.  All rights reserved.\n");
    wprintf(L"ExtractBundle sample\n\n");

    if (argc != 3)
    {
        wprintf(L"Usage:    ExtractBundle.exe inputFile outputPath\n");
        wprintf(L"    inputFile: Path to the bundle to extract\n");
        wprintf(L"    outputPath: Path to the folder to store extracted contents\n");
        return 2;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Create a bundle reader using the file name given in command line
        IAppxBundleReader* bundleReader = NULL;

        hr = GetBundleReader(argv[1], &bundleReader);

        // Print information about all footprint files, and extract them to disk
        if (SUCCEEDED(hr))
        {
            hr = ExtractFootprintFiles(bundleReader, argv[2]);
        }

        // Print information about all payload files, and extract them to disk
        if (SUCCEEDED(hr))
        {
            hr = ExtractPayloadPackages(bundleReader, argv[2]);
        }

        // Clean up allocated resources
        if (bundleReader != NULL)
        {
            bundleReader->Release();
            bundleReader = NULL;
        }
        CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nBundle extracted successfully.\n");
    }
    else
    {
        wprintf(L"\nBundle extraction failed with HRESULT 0x%08X.\n", hr);
    }
    return SUCCEEDED(hr) ? 0 : 1;
}
