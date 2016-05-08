//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

/*++

Module Name:

    ChangeMaxSize.cpp

Abstract:

    This module contains the implementation of how to change the max size of a 
    specified channel.

Environment:

    User-mode only.

--*/

#include <windows.h>
#include <winevt.h>
#include <stdio.h>
#include <stdlib.h>

ULONG
SetChannelMaxSize (
    __in PCWSTR ChannelPath,
    __in UINT64 MaxSize
    )

/*++

Routine Description:

    This function sets the max size in bytes for a given channel.

Parameters:

    ChannelPath - Suppiles the channel name.

    MaxSize - Suppiles the max size in bytes for the channel.

Return Value:

    A win32 error code indicating if max size was modified.

--*/

{
    EVT_HANDLE ChannelConfig;
    EVT_VARIANT Size;
    ULONG Status;

    //
    // Open the channel config store.
    //

    ChannelConfig = EvtOpenChannelConfig(NULL, ChannelPath, 0);
    if (ChannelConfig == NULL) {
        return GetLastError();
    }

    //
    // Set the channel max size property.
    // 

    Size.UInt64Val = MaxSize;
    Size.Type = EvtVarTypeUInt64;
    Size.Count = 1;

    if (EvtSetChannelConfigProperty(ChannelConfig,
                                    EvtChannelLoggingConfigMaxSize,
                                    0,
                                    &Size) == FALSE) {

        Status = GetLastError();
        goto Exit;
    }

    //
    // Try to save the config to data store.
    //

    if (EvtSaveChannelConfig(ChannelConfig, 0) == FALSE) {
        Status = GetLastError();
        goto Exit;
    }

    Status = ERROR_SUCCESS;

Exit:

    EvtClose(ChannelConfig);

    return Status;
}

void
ShowHelp (
    __in PCWSTR ExeFile
    )

/*++

Routine Description:

    This function displays the tool usage.

Parameters:

    ExeFile - Supplies the executable file name of this tool.

Return Value:

    None.

--*/

{
    wprintf(L"Usage: %s <ChannelName> <ChannelMaxSize>\n", ExeFile);
    wprintf(L"For Example: %s Microsoft-Windows-Eventlog/Debug 10240000\n", ExeFile);
}

int __cdecl
wmain (
    __in int argc,
    __in_ecount(argc) PWSTR* argv
    )

/*++

Routine Description:

    This function is the entry-point of this sample.

Arguments:

    argc - Supplies the number of command-line options.

    argv - Supplies the parsed command-line options.

Return Value:

    Win32 error code indicating the status of the execution of the tool.

--*/

{
    ULONG Status;

    if (argc < 3) {
        ShowHelp(argv[0]);
        return ERROR_SUCCESS;
    }

    Status = SetChannelMaxSize(argv[1], _wcstoui64(argv[2], NULL, 10));
    if (Status != ERROR_SUCCESS) {
        wprintf(L"Error: %u\n", Status);
    }

    return Status;
}
