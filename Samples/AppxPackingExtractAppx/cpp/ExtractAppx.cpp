// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx packaging APIs to read
// contents of an Appx package, and extract its contents to a folder on disk.

#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>

#include <AppxPackaging.h>  // For Appx Packaging APIs

#include "ExtractAppx.h"

// Types of footprint files in an Appx package
const int FootprintFilesCount = 4;
const APPX_FOOTPRINT_FILE_TYPE FootprintFilesType[FootprintFilesCount] = {
    APPX_FOOTPRINT_FILE_TYPE_MANIFEST,
    APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP,
    APPX_FOOTPRINT_FILE_TYPE_SIGNATURE,
    APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY
};
const LPCWSTR FootprintFilesName[FootprintFilesCount] = {
    L"manifest",
    L"block map",
    L"digital signature",
    L"code integrity catalog"
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
// Function to print some basic information about an IAppxFile, and write
// it to disk under the given path.
//
// Parameters:
// file - Instance of IAppxFile obtained from IAppxPackageReader representing
//        a footprint or payload file in the package.
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

    // String buffers obtained from the packaging APIs must be freed
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
// Function to extract all footprint files from a package reader.
//
// Parameters:
// packageReader - Instance of IAppxPackageReader over the Appx package whose
//                 footprint files are to be extracted.
// outputPath - Path of the folder where all extracted footprint files should
//              be placed.
//
HRESULT ExtractFootprintFiles(
    _In_ IAppxPackageReader* packageReader,
    _In_ LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    wprintf(L"\nExtracting footprint files from the package\n");

    for (int i = 0; SUCCEEDED(hr) && (i < FootprintFilesCount); i++)
    {
        IAppxFile* footprintFile = NULL;

        hr = packageReader->GetFootprintFile(FootprintFilesType[i], &footprintFile);

        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            // Some footprint files are optional, it is normal for the GetFootprintFile
            // call to fail when the file is not present.
            wprintf(L"\nThe package does not contain a %s.\n", FootprintFilesName[i]);
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
// Function to extract all payload files from a package reader.
//
// Parameters:
// packageReader - Instance of IAppxPackageReader over the Appx package whose
//                 payload files are to be extracted.
// outputPath - Path of the folder where all extracted payload files should be
//              placed.
//
HRESULT ExtractPayloadFiles(
    _In_ IAppxPackageReader* packageReader,
    _In_ LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    IAppxFilesEnumerator* payloadFiles = NULL;
    wprintf(L"\nExtracting payload files from the package\n");

    // Get an enumerator of all payload files from the package reader and iterate
    // through all files.
    hr = packageReader->GetPayloadFiles(&payloadFiles);

    if (SUCCEEDED(hr))
    {
        BOOL hasCurrent = FALSE;
        hr = payloadFiles->GetHasCurrent(&hasCurrent);

        while (SUCCEEDED(hr) && hasCurrent)
        {
            IAppxFile* payloadFile = NULL;

            hr = payloadFiles->GetCurrent(&payloadFile);

            if (SUCCEEDED(hr))
            {
                hr = ExtractFile(payloadFile, outputPath);
            }
            if (SUCCEEDED(hr))
            {
                hr = payloadFiles->MoveNext(&hasCurrent);
            }

            if (payloadFile != NULL)
            {
                payloadFile->Release();
                payloadFile = NULL;
            }
        }
    }

    if (payloadFiles != NULL)
    {
        payloadFiles->Release();
        payloadFiles = NULL;
    }
    return hr;
}

//
// Function to create an Appx package reader given the input file name.
//
// Parameters:
// inputFileName - Name including path to the Appx package (.appx file) to
//                 be opened.
// reader - Output parameter pointing to the created instance of
//          IAppxPackageReader when this function succeeds.
//
HRESULT GetPackageReader(
    _In_ LPCWSTR inputFileName,
    _Outptr_ IAppxPackageReader** reader)
{
    HRESULT hr = S_OK;
    IAppxFactory* appxFactory = NULL;
    IStream* inputStream = NULL;

    // Create a new Appx factory
    hr = CoCreateInstance(
            __uuidof(AppxFactory),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IAppxFactory),
            (LPVOID*)(&appxFactory));

    // Create a stream over the input Appx package
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

    // Create a new package reader using the factory.
    if (SUCCEEDED(hr))
    {
        hr = appxFactory->CreatePackageReader(
                inputStream,
                reader);
    }

    // Clean up allocated resources
    if (inputStream != NULL)
    {
        inputStream->Release();
        inputStream = NULL;
    }
    if (appxFactory != NULL)
    {
        appxFactory->Release();
        appxFactory = NULL;
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
    wprintf(L"ExtractAppx sample\n\n");

    if (argc != 3)
    {
        wprintf(L"Usage:    ExtractAppx.exe inputFile outputPath\n");
        wprintf(L"    inputFile: Path to the Appx package to extract\n");
        wprintf(L"    outputPath: Path to the folder to store extracted package contents\n");
        return 2;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Create a package reader using the file name given in command line
        IAppxPackageReader* packageReader = NULL;

        hr = GetPackageReader(argv[1], &packageReader);

        // Print information about all footprint files, and extract them to disk
        if (SUCCEEDED(hr))
        {
            hr = ExtractFootprintFiles(packageReader, argv[2]);
        }

        // Print information about all payload files, and extract them to disk
        if (SUCCEEDED(hr))
        {
            hr = ExtractPayloadFiles(packageReader, argv[2]);
        }

        // Clean up allocated resources
        if (packageReader != NULL)
        {
            packageReader->Release();
            packageReader = NULL;
        }
        CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nPackage extracted successfully.\n");
    }
    else
    {
        wprintf(L"\nPackage extraction failed with HRESULT 0x%08X.\n", hr);
    }
    return SUCCEEDED(hr) ? 0 : 1;
}
