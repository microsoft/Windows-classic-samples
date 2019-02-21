// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "UriSource.h"

using namespace winrt::Windows::Storage::Provider;

namespace winrt::CloudMirror::implementation
{
    void UriSource::GetPathForContentUri(hstring const& contentUri, Windows::Storage::Provider::StorageProviderGetPathForContentUriResult const& result)
    {
        result.Status(StorageProviderUriSourceStatus::FileNotFound);

        std::wstring prefix(L"http://cloudmirror.example.com/contentUri/");
        std::wstring uri(contentUri);
        if (0 == uri.compare(0, prefix.length(), prefix))
        {
            std::wstring localPath(ProviderFolderLocations::GetClientFolder());
            localPath.append(L"\\");
            localPath.append(uri.substr(prefix.length(), uri.find(L'?') - prefix.length()));

            if (PathFileExists(localPath.c_str()))
            {
                result.Path(localPath);
                result.Status(StorageProviderUriSourceStatus::Success);
            }
        }
    }

    void UriSource::GetContentInfoForPath(hstring const& path, Windows::Storage::Provider::StorageProviderGetContentInfoForPathResult const& result)
    {
        result.Status(StorageProviderUriSourceStatus::FileNotFound);
        PCWSTR fileName = PathFindFileName(path.c_str());
        std::wstring id(L"http://cloudmirror.example.com/contentId/");
        id.append(fileName);
        result.ContentId(id);

        id.assign(L"http://cloudmirror.example.com/contentUri/");
        id.append(fileName);
        id.append(L"?StorageProviderId=TestStorageProvider");
        result.ContentUri(id);

        result.Status(StorageProviderUriSourceStatus::Success);
    }
}
