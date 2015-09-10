// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx Bundle Manifest APIs
// to read metadata from an Appx Bundle on disk.

#pragma once

//
// Function to read a subset of attributes from the identity of either a
// bundle or a payload package in the bundle.
//
HRESULT ReadManifestIdentity(
    _In_ IAppxManifestPackageId* identity);

//
// Function to read a subset of attributes from the bundle manifest's
// <Packages> element.
//
HRESULT ReadManifestPackages(
    _In_ IAppxBundleManifestPackageInfoEnumerator* packages);

//
// Function to print a subset of information from a bundle manifest reader.
// Many methods in the bundle manifest API have similar usage patterns.
// Only a representative subset of API methods are shown here.
//
HRESULT ReadManifest(
    _In_ IAppxBundleManifestReader* manifestReader);

//
// Function to create an Appx Bundle reader given the input file name.
//
HRESULT GetBundleReader(
    _In_ LPCWSTR inputFileName,
    _Outptr_ IAppxBundleReader** reader);
