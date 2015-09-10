// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx package manifest and
// block map APIs to read metadata from an Appx package on disk.

#pragma once

//
// Function to read a subset of attributes from the manifest <Identity> element.
//
HRESULT ReadManifestPackageId(
    _In_ IAppxManifestPackageId* packageId);

//
// Function to read a subset of attributes from the manifest <Properties> element.
//
HRESULT ReadManifestProperties(
    _In_ IAppxManifestProperties* properties);

//
// Function to read a subset of attributes from the manifest <Applications> element.
//
HRESULT ReadManifestApplications(
    _In_ IAppxManifestApplicationsEnumerator* applications);

//
// Function to read attributes from the manifest <Resources> element.
//
HRESULT ReadManifestQualifiedResources(
    _In_ IAppxManifestQualifiedResourcesEnumerator* resources);

//
// Function to print a subset of information from a manifest reader.  Many
// methods in the manifest API have similar usage patterns.  To reduce
// redundancy, only a representative subset of manifest API methods are shown.
//
HRESULT ReadManifest(
    _In_ IAppxManifestReader* manifestReader);

//
// Function to print a subset of information from a block map reader
//
HRESULT ReadBlockMap(
    _In_ IAppxBlockMapReader* blockMapReader);

//
// Function to create an Appx package reader given the input file name.
//
HRESULT GetPackageReader(
    _In_ LPCWSTR inputFileName,
    _Outptr_ IAppxPackageReader** reader);
