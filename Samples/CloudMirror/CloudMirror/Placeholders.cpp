// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

//===============================================================
// Placeholders
//
//    Generates placeholders on the client to match what is on
//    the server.
//
// Fakery Factor:
//
//    Lotsa lyin' going on here. Since there is no cloud for this
//    sample, the Create method just walks the local folder that's
//    identified as the "cloud" and generates the placeholders along
//    with some custom states just because.
//
//===============================================================

void Placeholders::Create(
    _In_ PCWSTR sourcePath,
    _In_ PCWSTR sourceSubDirstr,
    _In_ PCWSTR destPath)
{
    try
    {
        WIN32_FIND_DATA findData;
        HANDLE hFileHandle;
        CF_PLACEHOLDER_CREATE_INFO cloudEntry;

        // make this either "subdirname\" or "" so that we can just always
        // append it without worry
        std::wstring sourceSubDir(sourceSubDirstr);
        if (sourceSubDir[0])
        {
            sourceSubDir.append(L"\\");
        }

        std::wstring fileName(sourcePath);
        fileName.append(sourceSubDir);
        fileName.append(L"\\*");

        hFileHandle =
            FindFirstFileEx(
                fileName.data(),
                FindExInfoStandard,
                &findData,
                FindExSearchNameMatch,
                NULL,
                FIND_FIRST_EX_ON_DISK_ENTRIES_ONLY);

        if (hFileHandle != INVALID_HANDLE_VALUE)
        {
            do 
            {
                if (!wcscmp(findData.cFileName, L".") ||
                    !wcscmp(findData.cFileName, L".."))
                {
                    continue;
                }

                std::wstring relativeName(sourceSubDir);
                relativeName.append(findData.cFileName);

                cloudEntry.FileIdentity = relativeName.data();
                cloudEntry.FileIdentityLength = (USHORT)(wcslen(relativeName.data()) + 1) * sizeof(WCHAR);

                cloudEntry.RelativeFileName = relativeName.data();
                cloudEntry.Flags = CF_PLACEHOLDER_CREATE_FLAG_MARK_IN_SYNC;
                cloudEntry.FsMetadata.FileSize.QuadPart = ((ULONGLONG)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;
                cloudEntry.FsMetadata.BasicInfo.FileAttributes = findData.dwFileAttributes;
                cloudEntry.FsMetadata.BasicInfo.CreationTime = Utilities::FileTimeToLargeInteger(findData.ftCreationTime);
                cloudEntry.FsMetadata.BasicInfo.LastWriteTime = Utilities::FileTimeToLargeInteger(findData.ftLastWriteTime);
                cloudEntry.FsMetadata.BasicInfo.LastAccessTime = Utilities::FileTimeToLargeInteger(findData.ftLastAccessTime);
                cloudEntry.FsMetadata.BasicInfo.ChangeTime = Utilities::FileTimeToLargeInteger(findData.ftLastWriteTime);

                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                {
                    cloudEntry.Flags |= CF_PLACEHOLDER_CREATE_FLAG_DISABLE_ON_DEMAND_POPULATION;
                    cloudEntry.FsMetadata.FileSize.QuadPart = 0;
                }

                try
                {
                    wprintf(L"Creating placeholder for %sx\n", relativeName.data());
                    winrt::check_hresult(CfCreatePlaceholders(destPath, &cloudEntry, 1, CF_CREATE_FLAG_NONE, NULL));
                }
                catch (...)
                {
                    // winrt::to_hresult() will eat the exception if it is a result of winrt::check_hresult,
                    // otherwise the exception will get rethrown and this method will crash out as it should
                    wprintf(L"Failed to create placeholder for %s with %08x\n", relativeName.data(), static_cast<HRESULT>(winrt::to_hresult()));
                    // Eating it here lets other files still get a chance. Not worth crashing the sample, but
                    // certainly noteworthy for production code
                    continue;
                }

                try
                {
                    winrt::StorageProviderItemProperty prop;
                    prop.Id(1);
                    prop.Value(L"Value1");
                    // This icon is just for the sample. You should provide your own branded icon here
                    prop.IconResource(L"shell32.dll,-44");

                    wprintf(L"Applying custom state for %sx\n", relativeName.data());
                    Utilities::ApplyCustomStateToPlaceholderFile(destPath, relativeName.data(), prop);

                    if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                    {
                        Create(sourcePath, relativeName.data(), destPath);
                    }
                }
                catch (...)
                {
                    // winrt::to_hresult() will eat the exception if it is a result of winrt::check_hresult,
                    // otherwise the exception will get rethrown and this method will crash out as it should
                    wprintf(L"Failed to set custom state on %s with %08x\n", relativeName.data(), static_cast<HRESULT>(winrt::to_hresult()));
                    // Eating it here lets other files still get a chance. Not worth crashing the sample, but
                    // certainly noteworthy for production code
                }

            } while (FindNextFile(hFileHandle, &findData));

            FindClose(hFileHandle);
        }
    }
    catch (...)
    {
        wprintf(L"Could not create cloud file placeholders in the sync root with %08x\n", static_cast<HRESULT>(winrt::to_hresult()));
        // Something weird enough happened that this is worth crashing out
        throw;
    }
}
