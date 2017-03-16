//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


#include "stdafx.h"
#include "FileHelper.h"

namespace fs = std::experimental::filesystem;


namespace DWriteCustomFontSets
{
    _Success_(return) bool FileHelper::GetApplicationPath(_Out_ std::wstring& applicationPath)
    {
        wchar_t buffer[MAX_PATH];
        if (!GetModuleFileName(nullptr, buffer, ARRAYSIZE(buffer)))
            return false;

        applicationPath.assign(buffer);
        size_t pos = applicationPath.find_last_of(L"\\/");
        if (pos == std::wstring::npos)
            return false;

        applicationPath.resize(pos + 1);
        return true;

    } // end FileHelper::GetApplicationPath()


    _Success_(return) bool FileHelper::GetFilesInSelectedFolder(const std::wstring& inFolderName, _Out_ std::vector<std::wstring>& selectedFilePaths)
    {
        // Start with an empty vector.
        selectedFilePaths.clear();

        // Early return if input is empty string.
        if (inFolderName.empty())
        {
            return false;
        }

        fs::path absPath = fs::system_complete(inFolderName);
        fs::file_status pathStatus = fs::status(absPath);

        // Confirm input path is a folder.
        if (pathStatus.type() != fs::file_type::directory)
        {
            return false;
        }

        // Add all the files in the folder to the vector.
        for (auto& file : fs::directory_iterator(absPath))
        {
            selectedFilePaths.push_back(file.path().wstring());
        }

        return true;

    } // end FileHelper::GetSelectedFiles()


    bool FileHelper::PathExists(const std::wstring& pathName)
    {
        fs::path path(pathName);
        return fs::exists(path);
    }

} // namespace DWriteCustomFontSets