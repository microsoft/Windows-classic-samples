// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "ThumbnailProvider.h"

// IInitializeWithItem
IFACEMETHODIMP ThumbnailProvider::Initialize(_In_ IShellItem* item, _In_ DWORD mode)
{
    try
    {
        winrt::check_hresult(item->QueryInterface(__uuidof(_itemDest), _itemDest.put_void()));

        // We want to identify the original item in the source folder that we're mirroring,
        // based on the placeholder item that we get initialized with.  There's probably a way
        // to do this based on the file identity blob but this just uses path manipulation.
        winrt::com_array<wchar_t> destPathItem;
        winrt::check_hresult(_itemDest->GetDisplayName(SIGDN_FILESYSPATH, winrt::put_abi(destPathItem)));

        wprintf(L"Thumbnail requested for %s\n", destPathItem.data());

        // Verify the item is underneath the root as we expect.
        if (!PathIsPrefix(ProviderFolderLocations::GetClientFolder(), destPathItem.data()))
        {
            return E_UNEXPECTED;
        }

        // Find the relative segment to the sync root.
        wchar_t relativePath[MAX_PATH];
        winrt::check_bool(PathRelativePathTo(relativePath, ProviderFolderLocations::GetClientFolder(), FILE_ATTRIBUTE_DIRECTORY, destPathItem.data(), FILE_ATTRIBUTE_NORMAL));

        // Now combine that relative segment with the original source folder, which results
        // in the path to the source item that we're mirroring.
        winrt::com_array<wchar_t> sourcePathItem;
        winrt::check_hresult(PathAllocCombine(ProviderFolderLocations::GetServerFolder(), relativePath, PATHCCH_ALLOW_LONG_PATHS, winrt::put_abi(sourcePathItem)));

        winrt::check_hresult(SHCreateItemFromParsingName(sourcePathItem.data(), nullptr, __uuidof(_itemSrc), _itemSrc.put_void()));
    }
    catch (...)
    {
        return winrt::to_hresult();
    }

    return S_OK;
}

// IThumbnailProvider
IFACEMETHODIMP ThumbnailProvider::GetThumbnail(_In_ UINT width, _Out_ HBITMAP* bitmap, _Out_ WTS_ALPHATYPE* alphaType)
{
    // Retrieve thumbnails of the placeholders on demand by delegating to the thumbnail of the source items.
    *bitmap = nullptr;
    *alphaType = WTSAT_UNKNOWN;

    try
    {
        winrt::com_ptr<IThumbnailProvider> thumbnailProviderSource;
        winrt::check_hresult(_itemSrc->BindToHandler(nullptr, BHID_ThumbnailHandler, __uuidof(thumbnailProviderSource), thumbnailProviderSource.put_void()));
        winrt::check_hresult(thumbnailProviderSource->GetThumbnail(width, bitmap, alphaType));
    }
    catch (...)
    {
        return winrt::to_hresult();
    }

    return S_OK;
}