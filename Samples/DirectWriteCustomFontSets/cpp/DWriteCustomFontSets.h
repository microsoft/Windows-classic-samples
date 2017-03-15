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
#include "CustomFontSetManager.h"



namespace DWriteCustomFontSets
{
    enum class Scenario // Enum used for control of which user-selected scenario is executed.
    {
        Undefined, // Set by default -- scenario is not specified or recognized.
        UnknownFontsInLocalFolder,
        KnownFontsInAppPackageFolder,
        KnownRemoteFonts,
        InMemoryFonts,
        PackedFonts
    };

    // Methods used by the app for displaying results after a font set has been created.
    void ReportFontProperties(const CustomFontSetManager& fontSetManager);
    void ReportFontDataDetails(const CustomFontSetManager& fontSetManager);

    // Other helper methods used in wmain.
    _Success_(return) bool GetFilesPathsInAppFontsFolder(_Out_ std::vector<std::wstring>& selectedFilePathNames);

} // namespace DWriteCustomFontSets
