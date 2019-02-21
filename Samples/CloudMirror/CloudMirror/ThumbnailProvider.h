// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include <thumbcache.h>

class __declspec(uuid("3d781652-78c5-4038-87a4-ec5940ab560a")) ThumbnailProvider : public winrt::implements<ThumbnailProvider, IInitializeWithItem, IThumbnailProvider>
{
public:
    // IInitializeWithItem
    IFACEMETHODIMP Initialize(_In_ IShellItem* item, _In_ DWORD mode);

    // IThumbnailProvider
    IFACEMETHODIMP GetThumbnail(_In_ UINT width, _Out_ HBITMAP* bitmap, _Out_ WTS_ALPHATYPE* alphaType);

private:
    winrt::com_ptr<IShellItem2> _itemDest;
    winrt::com_ptr<IShellItem2> _itemSrc;
};