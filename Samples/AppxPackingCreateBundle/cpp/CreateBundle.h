// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This is a simple application which uses the Appx Bundle APIs to produce
// an Appx Bundle from a list of app packages and resource packages on disk.

#pragma once

//
// Function to create a readable IStream over the file whose name is the
// concatenation of the path and fileName parameters.  For simplicity, file
// names including path are assumed to be 100 characters or less.  A real
// application should be able to handle longer names and allocate the
// necessary buffer dynamically.
//
HRESULT GetFileStream(
    _In_ LPCWSTR path,
    _In_ LPCWSTR fileName,
    _Outptr_ IStream** stream);

//
// Function to create an Appx Bundle writer with default settings, given the
// output file name.
//
HRESULT GetBundleWriter(
    _In_ LPCWSTR outputFileName,
    _Outptr_ IAppxBundleWriter** writer);
