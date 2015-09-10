// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx Bundle APIs to read the
// contents of an Appx bundle, and extract its contents to a folder on disk.

#pragma once

//
// Function to create a writable IStream over a file with the specified name
// under the specified path.  This function will also create intermediate
// subdirectories if necessary.  For simplicity, file names including path are
// assumed to be 200 characters or less.  A real application should be able to
// handle longer names and allocate the necessary buffer dynamically.
//
HRESULT GetOutputStream(
    _In_ LPCWSTR path,
    _In_ LPCWSTR fileName,
    _Outptr_ IStream** stream);

//
// Function to print some basic information about an IAppxFile object, and
// write it to disk under the given path.
//
HRESULT ExtractFile(
    _In_ IAppxFile* file,
    _In_ LPCWSTR outputPath);

//
// Function to extract all footprint files from a bundle reader.
//
HRESULT ExtractFootprintFiles(
    _In_ IAppxBundleReader* bundleReader,
    _In_ LPCWSTR outputPath);

//
// Function to extract all payload packages from a bundle reader.
//
HRESULT ExtractPayloadPackages(
    _In_ IAppxBundleReader* bundleReader,
    _In_ LPCWSTR outputPath);

//
// Function to create an Appx Bundle reader given the input file name.
//
HRESULT GetBundleReader(
    _In_ LPCWSTR inputFileName,
    _Outptr_ IAppxBundleReader** bundleReader);
