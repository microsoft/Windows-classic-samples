// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx Bundle Manifest APIs
// to read metadata from an Appx Bundle on disk.

#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>

#include <AppxPackaging.h>  // For Appx Bundle APIs

#include "DescribeBundle.h"

//
// Function to read a subset of attributes from the identity of either a bundle
// or a payload package in the bundle.
//
// Parameters:
// identity - Instance of IAppxManifestPackageId obtained from the bundle manifest
//            using either the IAppxBundleManifestReader::GetPackageId or the
//            IAppxBundleManifestPackageInfo::GetPackageId method.
//
HRESULT ReadManifestIdentity(
    _In_ IAppxManifestPackageId* identity)
{
    HRESULT hr = S_OK;
    LPWSTR packageFullName = NULL;
    LPWSTR packageName = NULL;
    UINT64 packageVersion = 0;

    hr = identity->GetPackageFullName(&packageFullName);

    if (SUCCEEDED(hr))
    {
        hr = identity->GetName(&packageName);
    }
    if (SUCCEEDED(hr))
    {
        hr = identity->GetVersion(&packageVersion);
    }
    if (SUCCEEDED(hr))
    {
        wprintf(L"Full name: %s\n", packageFullName);
        wprintf(L"Name: %s\n", packageName);
        wprintf(L"Version: ");

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
// Function to read a subset of attributes from the bundle manifest's
// <Packages> element.
//
// Parameters:
// packages - Instance of IAppxBundleManifestPackageInfoEnumerator obtained
//            from a bundle manifest using the
//            IAppxBundleManifestReader::GetPackageInfoItems method.
//
HRESULT ReadManifestPackages(
    _In_ IAppxBundleManifestPackageInfoEnumerator* packages)
{
    HRESULT hr = S_OK;
    BOOL hasCurrent = FALSE;
    UINT32 packagesCount = 0;

    hr = packages->GetHasCurrent(&hasCurrent);

    while (SUCCEEDED(hr) && hasCurrent)
    {
        IAppxBundleManifestPackageInfo* packageInfo = NULL;
        LPWSTR packageFileName = NULL;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
        IAppxManifestPackageId* packageIdentity = NULL;

        hr = packages->GetCurrent(&packageInfo);
        if (SUCCEEDED(hr))
        {
            hr = packageInfo->GetFileName(&packageFileName);
        }
        if (SUCCEEDED(hr))
        {
            packagesCount++;
            wprintf(L"Package #%u: %s\n", packagesCount, packageFileName);
        }

        // Read the payload package's type
        if (SUCCEEDED(hr))
        {
            hr = packageInfo->GetPackageType(&packageType);
        }
        if (SUCCEEDED(hr))
        {
            if (packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
            {
                wprintf(L"This is an app package.\n");
            }
            else if (packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE)
            {
                wprintf(L"This is a resource package.\n");
            }
        }

        // Read the payload package's identity
        if (SUCCEEDED(hr))
        {
            hr = packageInfo->GetPackageId(&packageIdentity);
        }
        if (SUCCEEDED(hr))
        {
            hr = ReadManifestIdentity(packageIdentity);
        }
        wprintf(L"\n");

        if (SUCCEEDED(hr))
        {
            hr = packages->MoveNext(&hasCurrent);
        }
        if (packageInfo != NULL)
        {
            packageInfo->Release();
            packageInfo = NULL;
        }
        if (packageIdentity != NULL)
        {
            packageIdentity->Release();
            packageIdentity = NULL;
        }
        CoTaskMemFree(packageFileName);
    }

    wprintf(L"The bundle contains %u package(s)\n", packagesCount);
    return hr;
}

//
// Function to print a subset of information from a bundle manifest reader.
// Many methods in the bundle manifest API have similar usage patterns.
// Only a representative subset of API methods are shown here.
//
// Parameters:
// manifestReader - Instance of IAppxBundleManifestReader created from a
//                  bundle manifest.
//
HRESULT ReadManifest(
    _In_ IAppxBundleManifestReader* manifestReader)
{
    HRESULT hr = S_OK;
    IAppxManifestPackageId* bundleId = NULL;
    IAppxBundleManifestPackageInfoEnumerator* packages = NULL;

    wprintf(L"\nReading <Identity> element\n");
    hr = manifestReader->GetPackageId(&bundleId);
    if (SUCCEEDED(hr))
    {
        hr = ReadManifestIdentity(bundleId);
    }

    wprintf(L"\nReading <Packages> element\n");
    if (SUCCEEDED(hr))
    {
        hr = manifestReader->GetPackageInfoItems(&packages);
    }
    if (SUCCEEDED(hr))
    {
        hr = ReadManifestPackages(packages);
    }

    // Clean up allocated resources
    if (bundleId != NULL)
    {
        bundleId->Release();
        bundleId = NULL;
    }
    if (packages != NULL)
    {
        packages->Release();
        packages = NULL;
    }

    return hr;
}

//
// Function to create an Appx Bundle reader given the input file name.
//
// Parameters:
// inputFileName - Path to the Appx Bundle (.appxbundle file) to be opened.
// reader - Output parameter pointing to the created instance of
//          IAppxBundleReader when this function succeeds.
//
HRESULT GetBundleReader(
    _In_ LPCWSTR inputFileName,
    _Outptr_ IAppxBundleReader** reader)
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

    // Create a stream over the input bundle file
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
                reader);
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
    wprintf(L"DescribeBundle sample\n\n");

    if (argc != 2)
    {
        wprintf(L"Usage:    DescribeBundle.exe inputFile\n");
        wprintf(L"    inputFile: Path to the Appx Bundle to read\n");
        return 2;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Create a bundle reader using the file name given in command line
        IAppxBundleReader* bundleReader = NULL;
        IAppxBundleManifestReader* manifestReader = NULL;

        hr = GetBundleReader(argv[1], &bundleReader);

        wprintf(L"\nReading the bundle manifest\n");

        // Get manifest reader for the bundle and read from the manifest
        if (SUCCEEDED(hr))
        {
            hr = bundleReader->GetManifest(&manifestReader);
        }
        if (SUCCEEDED(hr))
        {
            hr = ReadManifest(manifestReader);
        }

        // Clean up allocated resources
        if (manifestReader != NULL)
        {
            manifestReader->Release();
            manifestReader = NULL;
        }
        if (bundleReader != NULL)
        {
            bundleReader->Release();
            bundleReader = NULL;
        }
        CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nBundle metadata read successfully.\n");
    }
    else
    {
        wprintf(L"\nFailed to read bundle metadata with HRESULT 0x%08X.\n", hr);
    }
    return SUCCEEDED(hr) ? 0 : 1;
}
