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
#include "DWriteCustomFontSets.h"


namespace DWriteCustomFontSets
{
    enum class CommandLineArgument // Enum for recognized command line argument parameters, for use in a switch statement within ProcessArgs().
    {
        Help,
        Scenario
    };


    class CommandLineArgs
    {
    public:
        CommandLineArgs() {};

        void DisplayUsage() const;
        bool ProcessArgs(int argc, _In_reads_(argc) wchar_t** argv);
        Scenario GetScenario() const { return m_scenario; }

    private:
        Scenario m_scenario = Scenario::Undefined;

    }; // class CommandLineArgs

} // namespace DWriteCustomFontSets
