// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class Utilities
{
public:
    static void AddFolderToSearchIndexer(_In_ PCWSTR folder);
    static void ApplyTransferStateToFile(_In_ PCWSTR fullPath, _In_ CF_CALLBACK_INFO& callbackInfo, UINT64 total, UINT64 completed);
    static void ApplyCustomStateToPlaceholderFile(_In_ PCWSTR path, _In_ PCWSTR filename, _In_ winrt::StorageProviderItemProperty& prop);
    
    static winrt::com_array<wchar_t> ConvertSidToStringSid(_In_ PSID sid)
    {
        winrt::com_array<wchar_t> string;
        if (::ConvertSidToStringSid(sid, winrt::put_abi(string)))
        {
            return string;
        }
        else
        {
            throw std::bad_alloc();
        }
    };

    inline static LARGE_INTEGER FileTimeToLargeInteger(_In_ const FILETIME fileTime)
    {
        LARGE_INTEGER largeInteger;

        largeInteger.LowPart = fileTime.dwLowDateTime;
        largeInteger.HighPart = fileTime.dwHighDateTime;

        return largeInteger;
    }

    inline static LARGE_INTEGER LongLongToLargeInteger(_In_ const LONGLONG longlong)
    {
        LARGE_INTEGER largeInteger;
        largeInteger.QuadPart = longlong;
        return largeInteger;
    }

    inline static CF_OPERATION_INFO ToOperationInfo(
        _In_ CF_CALLBACK_INFO const* info,
        _In_ CF_OPERATION_TYPE operationType)
    {
        return
            CF_OPERATION_INFO
        {
            sizeof(CF_OPERATION_INFO),
            operationType,
            info->ConnectionKey,
            info->TransferKey
        };
    }
};

