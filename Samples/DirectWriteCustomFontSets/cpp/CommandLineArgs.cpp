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
#include "CommandLineArgs.h"


namespace DWriteCustomFontSets
{
    void CommandLineArgs::DisplayUsage() const
    {
        std::wcout <<
            L"DWriteCustomFontSets Sample\n"
            L"\n"
            L"Creates a custom font set using DirectWrite and displays basic properties of the\n"
            L"fonts.\n"
            L"\n"
            L"Usage:\n"
            L"\n"
            L"DWriteCustomFontSets [/?]\n"
            L"  Displays this usage information.\n"
            L"\n"
            L"DWriteCustomFontSets /s Scenario\n"
            L"  Executes the sample code for the specified scenario.\n"
            L"\n"
            LR"(Scenarios: "1" | "2" | "3" | "4" | "5")" L"\n"
            L"\n"
            L"Example:"
            L"  DWriteCustomFontSets /s 1\n"
            L"\n"
            L"The sample demonstrates code implementation for the following representative\n"
            L"scenarios:\n"
            L"\n"
            L"  1  Creates a custom font set using font files in a local file folder. The fonts\n"
            L"     are not assumed to be known by the app, and all font properties used in the\n"
            L"     font set are extracted directly from the font files as they are added to the\n"
            L"     set. Requires any Windows 10 version (build 10240 or later). On Windows 10 \n"
            L"     Creators Update (preview build 15021 or later), a different implementation\n"
            L"     with newer, recommended APIs is used than on earlier Windows 10 versions.\n"
            L"\n"
            L"  2  Creates a custom font set using a set of known fonts contained in the app.\n"
            L"     The app assigns custom properties to each font. Requires any Windows 10\n"
            L"     version.\n"
            L"\n"
            L"  3  Creates a custom font set using remote fonts from the Web. This may be\n"
            L"     useful, for example, for creativity apps that use a collection of fonts\n"
            L"     stored in a cloud font service. The remote fonts are assumed to be known,\n"
            L"     and custom font properties are used, allowing the custom font set to be\n"
            L"     created without needing to download the font data beforehand. This scenario\n"
            L"     also demonstrates how to interact with DirectWrite APIs that handle\n"
            L"     downloading of the fonts as the actual font data is needed. Requires\n"
            L"     Windows 10 Creators Update (preview build 15021 or later).\n"
            L"\n"
            L"     Note: Since actual download of remote fonts has unpredictable latency or\n"
            L"     certainty of success, the sample will time out after 15 seconds.\n"
            L"\n"
            L"  4  Creates a custom font set using an in-memory buffer containing font data.\n"
            L"     The sample illustrates two specific scenarios: fonts embedded within the\n"
            L"     app binary as a resource, and fonts embedded in a document file. But the\n"
            L"     implementation can also be useful for any other situation in which font\n"
            L"     data may be loaded in memory. Requires Windows 10 Creators Update.\n"
            L"\n"
            L"  5  Creates a custom font set using packed, WOFF2 font data. This scenario\n"
            L"     demonstrates use of new APIs for unpacking font data that is packaged in\n"
            L"     WOFF or WOFF2 formats. Requires Windows 10 Creators Update.\n"
            L"\n"
            ;
    } // DisplayUsage


    // Map used to map command line arguments to an enum (for use in a switch statement).
    std::map<std::wstring, CommandLineArgument> ArgumentMap = 
    {
        { L"?", CommandLineArgument::Help},
        { L"s", CommandLineArgument::Scenario},
        { L"S", CommandLineArgument::Scenario}
    };


    // Map used to map scenario IDs in the command line to an enum.
    std::map<std::wstring, Scenario> ScenarioMap =
    {
        { L"1", Scenario::UnknownFontsInLocalFolder },
        { L"2", Scenario::KnownFontsInAppPackageFolder },
        { L"3", Scenario::KnownRemoteFonts },
        { L"4", Scenario::InMemoryFonts },
        { L"5", Scenario::PackedFonts}
    };


    bool CommandLineArgs::ProcessArgs(int argc, _In_reads_(argc) wchar_t** argv)
    {
        // Returns false if there was a problem in the command line arguments, or if "/?" was entered. Otherwise,
        // returns true (valid command line arguments, not requesting usage info).

        // Process command line arguments. Ignore the first one (i == 0), which is always the pathname to the command executable.
        for (int i = 1; i < argc; i++)
        {
            wchar_t* arg = argv[i];

            // Parameter tokens must begin with '/' or '-'.
            if (arg[0] != L'/' && arg[0] != L'-')
            {
                return false;
            }

            // We've got a candidate token. Validate and process.

            // strip prefix from arg
            std::wstring token(arg + 1);

            // Lookup the token in the ArgumentMap to recognize value arguments.
            CommandLineArgument argument;
            auto mapArgumentPair = ArgumentMap.find(token);
            if (mapArgumentPair == ArgumentMap.end())
            {
                // The token wasn't found in the map, hence is invalid.
                return false;
            }
            argument = mapArgumentPair->second;

            switch (argument)
            {
            case CommandLineArgument::Scenario:
            {
                // In this case, a second token for the scenario ID is required.

                if (i + 1 == argc)
                {
                    // Out of tokens!
                    return false;
                }
                i++;
                auto mapScenarioPair = ScenarioMap.find(std::wstring(argv[i]));
                if (mapScenarioPair == ScenarioMap.end())
                {
                    // The token wasn't found in the map, hence is invalid.
                    return false;
                }
                m_scenario = mapScenarioPair->second;
                break;
            }

            case CommandLineArgument::Help:
                // Treat this like an error case -- intentional fallthrough to the default case.
            default:
                return false;

            } // end switch

        } // for loop

        // Check for expected arguments.
        if (m_scenario == Scenario::Undefined)
        {
            return false;
        }

        return true;
    } // ProcessArgs

} // namespace DWriteCustomFontSets