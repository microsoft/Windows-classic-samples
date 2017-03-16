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


#pragma once

#include "stdafx.h"


namespace DWriteCustomFontSets
{
    class FileHelper
    {
    public:
        _Success_(return) static bool GetApplicationPath(_Out_ std::wstring& applicationPath);
        _Success_(return) static bool GetFilesInSelectedFolder(const std::wstring& inFolderName, _Out_ std::vector<std::wstring>& selectedFilePaths);
        static bool PathExists(const std::wstring& pathName);

    }; // class FileHelper

} // namespace DWriteCustomFontSets
