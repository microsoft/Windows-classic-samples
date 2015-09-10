// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx manifest and block map
// APIs to read metadata from an Appx package.

#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>

#include <AppxPackaging.h>  // For Appx Packaging APIs

#include "DescribeAppx.h"

//
// Function to read a subset of attributes from the manifest <Identity> element.
//
// Parameters:
// packageId - Instance of IAppxManifestPackageId obtained from a package
//             manfest using the IAppxManifestReader::GetPackageId method.
//
HRESULT ReadManifestPackageId(
    _In_ IAppxManifestPackageId* packageId)
{
    HRESULT hr = S_OK;
    LPWSTR packageFullName = NULL;
    LPWSTR packageName = NULL;
    UINT64 packageVersion = 0;

    hr = packageId->GetPackageFullName(&packageFullName);

    if (SUCCEEDED(hr))
    {
        hr = packageId->GetName(&packageName);
    }
    if (SUCCEEDED(hr))
    {
        hr = packageId->GetVersion(&packageVersion);
    }
    if (SUCCEEDED(hr))
    {
        wprintf(L"Package full name: %s\n", packageFullName);
        wprintf(L"Package name: %s\n", packageName);
        wprintf(L"Package version: ");

        // Convert version number from 64-bit integer to dot-quad form
        for (int bitPosition = 0x30; bitPosition >= 0; bitPosition -= 0x10)
        {
            UINT64 versionWord = (packageVersion >> bitPosition) & 0xFFFF;
            wprintf(L"%llu.", versionWord);
        }
        wprintf(L"\n");
    }

    // Free all string buffers returned from the manifest API
    CoTaskMemFree(packageFullName);
    CoTaskMemFree(packageName);
    return hr;
}

//
// Function to read a subset of attributes from the manifest <Properties> element.
//
// Parameters:
// properties - Instance of IAppxManifestProperties obtained from a package manifest
//            using the IAppxManifestReader::GetProperties method.
//
HRESULT ReadManifestProperties(
    _In_ IAppxManifestProperties* properties)
{
    HRESULT hr = S_OK;
    LPWSTR displayName = NULL;
    LPWSTR description = NULL;

    hr = properties->GetStringValue(L"DisplayName", &displayName);

    if (SUCCEEDED(hr))
    {
        hr = properties->GetStringValue(L"Description", &description);
    }
    if (SUCCEEDED(hr))
    {
        wprintf(L"Display name: %s\n", displayName);
        wprintf(L"Description: %s\n", description);
    }

    // Free all string buffers returned from the manifest API
    CoTaskMemFree(displayName);
    CoTaskMemFree(description);
    return hr;
}

//
// Function to read a subset of attributes from the manifest <Applications> element.
//
// Parameters:
// applications - Instance of IAppxManifestApplicationsEnumerator obtained from a
//                package manifest using the IAppxManifestReader::GetApplications
//                method.
//
HRESULT ReadManifestApplications(
    _In_ IAppxManifestApplicationsEnumerator* applications)
{
    HRESULT hr = S_OK;
    BOOL hasCurrent = FALSE;
    UINT32 applicationsCount = 0;

    hr = applications->GetHasCurrent(&hasCurrent);

    while (SUCCEEDED(hr) && hasCurrent)
    {
        IAppxManifestApplication* application = NULL;
        LPWSTR applicationName = NULL;

        hr = applications->GetCurrent(&application);
        if (SUCCEEDED(hr))
        {
            application->GetStringValue(L"DisplayName", &applicationName);
        }
        if (SUCCEEDED(hr))
        {
            applicationsCount++;
            wprintf(L"Application #%u: %s\n", applicationsCount, applicationName);
        }

        if (SUCCEEDED(hr))
        {
            hr = applications->MoveNext(&hasCurrent);
        }
        if (application != NULL)
        {
            application->Release();
            application = NULL;
        }
        CoTaskMemFree(applicationName);
    }

    wprintf(L"Package contains %u application(s)\n", applicationsCount);
    return hr;
}

//
// Function to read attributes from the manifest <Resources> element.
//
// Parameters:
// resources - Instance of IAppxManifestQualifiedResourcesEnumerator obtained from a
//             package manifest using the IAppxManifestReader2::GetQualifiedResources
//             method.
//
// Note: IAppxManifestReader2::GetQualifiedResources differs from the 
// IAppxManifestReader::GetResources method in that the latter returns an
// IAppxManifestResourcesEnumerator object, which can only enumerate over
// Language attributes specified on a Resource element. To read all the
// attributes on the Resource element we need to use the
// IAppxManifestReader2::GetQualifiedResources method.
//
HRESULT ReadManifestQualifiedResources(
    _In_ IAppxManifestQualifiedResourcesEnumerator* resources)
{
    HRESULT hr = S_OK;
    BOOL hasCurrent = FALSE;
    UINT32 resourcesCount = 0;

    hr = resources->GetHasCurrent(&hasCurrent);

    while (SUCCEEDED(hr) && hasCurrent)
    {
        IAppxManifestQualifiedResource* resource = NULL;
        LPWSTR resourceLanguage = NULL;
        LPWSTR dxFeatureLevelString = NULL;
        UINT32 resourceScale = 0;
        DX_FEATURE_LEVEL dxFeatureLevel = DX_FEATURE_LEVEL_UNSPECIFIED;

        hr = resources->GetCurrent(&resource);
        if (SUCCEEDED(hr))
        {
            resourcesCount++;
            wprintf(L"Resource #%u: ", resourcesCount);

            // Resource elements can only specify one resource qualifier
            // value at a time. The code below checks to see which of the
            // following three qualifiers is specified - 'Language', 'Scale'
            // or 'DXFeatureLevel - and prints its value.

            // Check if 'Language' is specified.
            hr = resource->GetLanguage(&resourceLanguage);
            if (SUCCEEDED(hr))
            {
                if (resourceLanguage != NULL)
                {
                    wprintf(L"Language:%s\n", resourceLanguage);
                }
                else
                {
                    // Check if 'Scale' is specified.
                    hr = resource->GetScale(&resourceScale);
                    if (SUCCEEDED(hr))
                    {
                        if (resourceScale != 0)
                        {
                            wprintf(L"Scale:%u\n", resourceScale);
                        }
                        else
                        {
                            // Check if 'DXFeatureLevel' is specified.
                            hr = resource->GetDXFeatureLevel(&dxFeatureLevel);
                            if (SUCCEEDED(hr))
                            {
                                switch(dxFeatureLevel)
                                {
                                    case DX_FEATURE_LEVEL_9:
                                        dxFeatureLevelString = L"DX_FEATURE_LEVEL_9";
                                        break;
                                    case DX_FEATURE_LEVEL_10:
                                        dxFeatureLevelString = L"DX_FEATURE_LEVEL_10";
                                        break;
                                    case DX_FEATURE_LEVEL_11:
                                        dxFeatureLevelString = L"DX_FEATURE_LEVEL_11";
                                        break;
                                    default:
                                        hr = E_UNEXPECTED;
                                }
                                if (SUCCEEDED(hr))
                                {
                                    wprintf(L"DXFeatureLevel:%s\n", dxFeatureLevelString);
                                }
                            }
                        }
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = resources->MoveNext(&hasCurrent);
        }
        if (resource != NULL)
        {
            resource->Release();
            resource = NULL;
        }
        CoTaskMemFree(resourceLanguage);
    }

    wprintf(L"Package contains %u resource(s)\n", resourcesCount);
    return hr;
}

//
// Function to print a subset of information from a manifest reader.  Many
// methods in the manifest API have similar usage patterns.  To reduce
// redundancy, only a representative subset of manifest API methods are shown.
//
// Parameters:
// manifestReader - Instance of IAppxManifestReader created from an Appx packge
//                  manifest, containing the manifest data to be read.
//
HRESULT ReadManifest(
    _In_ IAppxManifestReader* manifestReader)
{
    HRESULT hr = S_OK;
    IAppxManifestPackageId* packageId = NULL;
    IAppxManifestProperties* properties = NULL;
    IAppxManifestApplicationsEnumerator* applications = NULL;
    IAppxManifestReader2* manifestReader2 = NULL;
    IAppxManifestQualifiedResourcesEnumerator* resources = NULL;

    wprintf(L"\nReading manifest\n");

    // Get elements and attributes from the manifest reader
    wprintf(L"\nReading <Identity> element\n");
    hr = manifestReader->GetPackageId(&packageId);
    if (SUCCEEDED(hr))
    {
        hr = ReadManifestPackageId(packageId);
    }

    wprintf(L"\nReading <Properties> element\n");
    if (SUCCEEDED(hr))
    {
        hr = manifestReader->GetProperties(&properties);
    }
    if (SUCCEEDED(hr))
    {
        hr = ReadManifestProperties(properties);
    }

    wprintf(L"\nReading <Application> elements\n");
    if (SUCCEEDED(hr))
    {
        hr = manifestReader->GetApplications(&applications);
    }
    if (SUCCEEDED(hr))
    {
        hr = ReadManifestApplications(applications);
    }

    wprintf(L"\nReading <Resource> elements\n");
    // QueryInterface for the IAppxManifestReader2 interface
    if (SUCCEEDED(hr))
    {
        hr = manifestReader->QueryInterface(__uuidof(IAppxManifestReader2), (void**)&manifestReader2);
    }
    if (SUCCEEDED(hr))
    {
        hr = manifestReader2->GetQualifiedResources(&resources);
    }
    if (SUCCEEDED(hr))
    {
        hr = ReadManifestQualifiedResources(resources);
    }

    // Clean up allocated resources
    if (packageId != NULL)
    {
        packageId->Release();
        packageId = NULL;
    }
    if (properties != NULL)
    {
        properties->Release();
        properties = NULL;
    }
    if (applications != NULL)
    {
        applications->Release();
        applications = NULL;
    }
    if (resources != NULL)
    {
        resources->Release();
        resources = NULL;
    }
    if (manifestReader2 != NULL)
    {
        manifestReader2->Release();
        manifestReader2 = NULL;
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nManifest read successfully.\n");
    }
    else
    {
        wprintf(L"\nFailed to read manifest with HRESULT 0x%08X.\n", hr);
    }
    return hr;
}

//
// Function to print a subset of information from a block map reader
//
// Parameters:
// blockMapReader - Instance of IAppxBlockMapReader created from an Appx
//                  block map, containing the block map data to be read.
//
HRESULT ReadBlockMap(
    _In_ IAppxBlockMapReader* blockMapReader)
{
    HRESULT hr = S_OK;
    IAppxBlockMapFilesEnumerator* blockMapFiles = NULL;
    BOOL hasCurrentFile = FALSE;
    UINT32 fileCount = 0;

    wprintf(L"\nReading block map\n");

    // Get an enumerator of all files in the block map and enumerate through
    // them all.
    hr = blockMapReader->GetFiles(&blockMapFiles);

    if (SUCCEEDED(hr))
    {
        hr = blockMapFiles->GetHasCurrent(&hasCurrentFile);
    }
    while (SUCCEEDED(hr) && hasCurrentFile)
    {
        IAppxBlockMapFile* file = NULL;
        LPWSTR fileName = NULL;
        UINT64 uncompressedSize = 0;
        UINT64 compressedSize = 0;
        UINT32 blockCount = 0;

        IAppxBlockMapBlocksEnumerator* blocks = NULL;
        BOOL hasCurrentBlock = FALSE;

        hr = blockMapFiles->GetCurrent(&file);
        fileCount++;

        // Get name and uncompressed size from the block map file item
        if (SUCCEEDED(hr))
        {
            hr = file->GetName(&fileName);
        }
        if (SUCCEEDED(hr))
        {
            hr = file->GetUncompressedSize(&uncompressedSize);
        }

        // Get total compressed size of file by iterating through all blocks
        // of the file and adding the compressed size of each block
        if (SUCCEEDED(hr))
        {
            hr = file->GetBlocks(&blocks);
        }
        if (SUCCEEDED(hr))
        {
            hr = blocks->GetHasCurrent(&hasCurrentBlock);
        }
        while (SUCCEEDED(hr) && hasCurrentBlock)
        {
            IAppxBlockMapBlock* block = NULL;
            UINT32 blockCompressedSize = 0;

            hr = blocks->GetCurrent(&block);

            if (SUCCEEDED(hr))
            {
                hr = block->GetCompressedSize(&blockCompressedSize);
                compressedSize += blockCompressedSize;
            }

            if (SUCCEEDED(hr))
            {
                hr = blocks->MoveNext(&hasCurrentBlock);
                blockCount++;
            }
            if (block != NULL)
            {
                block->Release();
                block = NULL;
            }
        }

        if (SUCCEEDED(hr))
        {
            wprintf(L"\nFile #%u\n", fileCount);
            wprintf(L"Name: %s\n", fileName);
            wprintf(L"Uncompressed size: %llu bytes\n", uncompressedSize);
            wprintf(L"Stored in %u block(s)\n", blockCount);
            wprintf(L"Compressed size: %llu bytes\n", compressedSize);
        }
        if (SUCCEEDED(hr))
        {
            hr = blockMapFiles->MoveNext(&hasCurrentFile);
        }

        // Clean up allocated resources
        if (blocks != NULL)
        {
            blocks->Release();
            blocks = NULL;
        }
        if (file != NULL)
        {
            file->Release();
            file = NULL;
        }
        CoTaskMemFree(fileName);
    }
    wprintf(L"\nThe block map contains %u file(s).\n", fileCount);

    if (blockMapFiles != NULL)
    {
        blockMapFiles->Release();
        blockMapFiles = NULL;
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nBlock map read successfully.\n");
    }
    else
    {
        wprintf(L"\nFailed to read block map with HRESULT 0x%08X.\n", hr);
    }
    return hr;
}

//
// Function to create an Appx package reader given the input file name.
//
// Parameters:
// inputFileName - Path to the Appx package (.appx file) to be opened.
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
    wprintf(L"DescribeAppx sample\n\n");

    if (argc != 2)
    {
        wprintf(L"Usage:    DescribeAppx.exe inputFile\n");
        wprintf(L"    inputFile: Path to the Appx package to read\n");
        return 2;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Create a package reader using the file name given in command line
        IAppxPackageReader* packageReader = NULL;
        IAppxManifestReader* manifestReader = NULL;
        IAppxBlockMapReader* blockMapReader = NULL;

        hr = GetPackageReader(argv[1], &packageReader);

        // Get manifest reader for the package and read from the manifest
        if (SUCCEEDED(hr))
        {
            hr = packageReader->GetManifest(&manifestReader);
        }
        if (SUCCEEDED(hr))
        {
            hr = ReadManifest(manifestReader);
        }

        // Get block map reader for the package and read from the block map
        if (SUCCEEDED(hr))
        {
            hr = packageReader->GetBlockMap(&blockMapReader);
        }
        if (SUCCEEDED(hr))
        {
            hr = ReadBlockMap(blockMapReader);
        }

        // Clean up allocated resources
        if (blockMapReader != NULL)
        {
            blockMapReader->Release();
            blockMapReader = NULL;
        }
        if (manifestReader != NULL)
        {
            manifestReader->Release();
            manifestReader = NULL;
        }
        if (packageReader != NULL)
        {
            packageReader->Release();
            packageReader = NULL;
        }
        CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nPackage metadata read successfully.\n");
    }
    else
    {
        wprintf(L"\nFailed to read package metadata with HRESULT 0x%08X.\n", hr);
    }
    return SUCCEEDED(hr) ? 0 : 1;
}
