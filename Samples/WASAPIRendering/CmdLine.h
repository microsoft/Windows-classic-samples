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

//
//  Command line parsing logic
//
struct CommandLineSwitch
{
    enum CommandLineSwitchType
    {
        SwitchTypeNone,
        SwitchTypeInteger,
        SwitchTypeString,
    };

    LPCWSTR SwitchName;
    LPCWSTR SwitchHelp;
    CommandLineSwitchType SwitchType;
    void* SwitchValue;
    bool SwitchValueOptional;
};

bool ParseCommandLine(int argc, wchar_t* argv[], const CommandLineSwitch Switches[], size_t SwitchCount);
