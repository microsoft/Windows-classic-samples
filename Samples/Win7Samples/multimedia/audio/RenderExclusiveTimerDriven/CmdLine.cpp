// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#include "StdAfx.h"
#include "CmdLine.h"

//
//  Parse the command line and set options based on those arguments.
//
bool ParseCommandLine(int argc, wchar_t *argv[], const CommandLineSwitch Switches[], size_t SwitchCount)
{
    //
    //  Iterate over the command line arguments
    for (int i = 1 ; i < argc ; i += 1)
    {
        if (argv[i][0] == L'-' || argv[i][0] == L'/')
        {
            size_t switchIndex;
            for (switchIndex = 0 ; switchIndex < SwitchCount ; switchIndex += 1)
            {
                size_t switchNameLength = wcslen(Switches[switchIndex].SwitchName);
                if (_wcsnicmp(&argv[i][1], Switches[switchIndex].SwitchName, switchNameLength) == 0 && 
                    (argv[i][switchNameLength+1]==L':' || argv[i][switchNameLength+1] == '\0'))
                {
                    wchar_t *switchValue = NULL;

                    if (Switches[switchIndex].SwitchType != CommandLineSwitch::SwitchTypeNone)
                    {
                        //
                        //  This is a switch value that expects an argument.
                        //
                        //  Check to see if the last character of the argument is a ":".
                        //
                        //  If it is, then the user specified an argument, so we should use the value after the ":"
                        //  as the argument.
                        //
                        if (argv[i][switchNameLength+1] == L':')
                        {
                            switchValue = &argv[i][switchNameLength+2];
                        }
                        else if (i < argc)
                        {
                            //
                            //  If the switch value isn't optional, the next argument
                            //  must be the value.
                            //
                            if (!Switches[switchIndex].SwitchValueOptional)
                            {
                                switchValue = argv[i+1];
                                i += 1; // Skip the argument.
                            }
                            //
                            //  Otherwise the switch value is optional, so check the next parameter.
                            //
                            //  If it's a switch, the user didn't specify a value, if it's not a switch
                            //  the user DID specify a value.
                            //
                            else if (argv[i+1][0] != L'-' && argv[i+1][0] != L'/')
                            {
                                switchValue = argv[i+1];
                                i += 1; // Skip the argument.
                            }
                        }
                        else if (!Switches[switchIndex].SwitchValueOptional)
                        {
                            printf("Invalid command line argument parsing option %S\n", Switches[switchIndex].SwitchName);
                            return false;
                        }
                    }
                    switch (Switches[switchIndex].SwitchType)
                    {
                        //
                        //  SwitchTypeNone switches take a boolean parameter indiating whether or not the parameter was present.
                        //
                    case CommandLineSwitch::SwitchTypeNone:
                        *reinterpret_cast<bool *>(Switches[switchIndex].SwitchValue) = true;
                        break;
                        //
                        //  SwitchTypeInteger switches take an integer parameter.
                        //
                    case CommandLineSwitch::SwitchTypeInteger:
                        {
                            wchar_t *endValue;
                            long value = wcstoul(switchValue, &endValue, 0);
                            if (value == ULONG_MAX || value == 0 || (*endValue != L'\0' && !iswspace(*endValue)))
                            {
                                printf("Command line switch %S expected an integer value, received %S", Switches[switchIndex].SwitchName, switchValue);
                                return false;
                            }
                            *reinterpret_cast<long *>(Switches[switchIndex].SwitchValue) = value;
                            break;
                        }
                        //
                        //  SwitchTypeString switches take a string parameter - allocate a buffer for the string using operator new[].
                        //
                    case CommandLineSwitch::SwitchTypeString:
                        {
                            wchar_t ** switchLocation = reinterpret_cast<wchar_t **>(Switches[switchIndex].SwitchValue);
                            //
                            //  If the user didn't specify a value, set the location to NULL.
                            //
                            if (switchValue == NULL || *switchValue == '\0')
                            {
                                *switchLocation = NULL;
                            }
                            else
                            {
                                size_t switchLength = wcslen(switchValue)+1;
                                *switchLocation = new (std::nothrow) wchar_t[switchLength];
                                if (*switchLocation == NULL)
                                {
                                    printf("Unable to allocate memory for switch %S", Switches[switchIndex].SwitchName);
                                    return false;
                                }

                                HRESULT hr = StringCchCopy(*switchLocation, switchLength, switchValue);
                                if (FAILED(hr))
                                {
                                    printf("Unable to copy command line string %S to buffer\n", switchValue);
                                    return false;
                                }
                            }
                            break;
                        }
                    default:
                        break;
                    }
                    //  We've processed this command line switch, we can move to the next argument.
                    //
                    break;
                }
            }
            if (switchIndex == SwitchCount)
            {
                printf("unrecognized switch: %S", argv[i]);
                return false;
            }
        }
    }
    return true;
}
