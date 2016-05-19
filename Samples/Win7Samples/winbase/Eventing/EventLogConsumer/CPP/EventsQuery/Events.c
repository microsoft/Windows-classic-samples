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

    events.c

Abstract:

    This module contains the implementation of a simple tool to enumerate
    event log channels and to query & filter events from channels.

Environment:

    User-mode only.

--*/

#include <windows.h>
#include <winevt.h>
#include <stdio.h>
#include <stdlib.h>

//
// Disable the nonstandard extension used struct in winevt.h since 
// winevt.h assume the users are CPP file intead of C file.
//

#pragma warning(disable:4201)

ULONG
EnumerateChannels (
    VOID
    )

/*++

Routine Description:

    This function enumerates all the EventLog channels and prints their names
    to the standard output.

Arguments:

    None.

Return Value:

    Win32 error code indicating if enumeration was successful.

--*/

{
    PWSTR Buffer;
    ULONG BufferLength;
    ULONG BufferLengthNeeded;
    EVT_HANDLE ChannelEnum;
    ULONG Status;

    //
    // Create the channel enumeration handle.
    //

    ChannelEnum = EvtOpenChannelEnum(NULL, 0);
    if (ChannelEnum == NULL) {
        return GetLastError();
    }

    Buffer = NULL;
    BufferLength = 0;
    BufferLengthNeeded = 0;

    do {

        //
        // Expand the buffer size if needed.
        //

        if (BufferLengthNeeded > BufferLength) {
            free(Buffer);
            BufferLength = BufferLengthNeeded;
            Buffer = malloc(BufferLength * sizeof(WCHAR));
            if (Buffer == NULL) {
                Status = ERROR_OUTOFMEMORY;
                break;
            }
        }

        //
        // Try to get the next channel name.
        //

        if (EvtNextChannelPath(ChannelEnum,
                               BufferLength,
                               Buffer,
                               &BufferLengthNeeded) == FALSE) {
            Status = GetLastError();
        } else {
            Status = ERROR_SUCCESS;
            wprintf(L"%s\n", Buffer);
        }

    } while ((Status == ERROR_SUCCESS) || 
             (Status == ERROR_INSUFFICIENT_BUFFER));

    //
    // Free all resources associated with channel enumeration.
    //

    free(Buffer);

    EvtClose(ChannelEnum);

    //
    // When EvtNextChannelPath returns ERROR_NO_MORE_ITEMS, we have actually
    // iterated through all the channels and thus succeeded.
    //

    if (Status == ERROR_NO_MORE_ITEMS) {
        Status = ERROR_SUCCESS;
    }

    return Status;
}

ULONG
QueryEvents (
    __in PCWSTR Channel,
    __in PCWSTR XPath
    )

/*++

Routine Description:

    This function queries events from the given channel and prints their
    description to the standard output.

Arguments:

    Channel - Supplies the name of the channel whose events will be displayed.

    XPath - Supplies the XPath expression to filter events with.

Return Value:

    Win32 error code indicating if querying was successful.

--*/

{
    PWSTR Buffer;
    ULONG BufferSize;
    ULONG BufferSizeNeeded;
    ULONG Count;
    EVT_HANDLE Event;
    EVT_HANDLE Query;
    ULONG Status;

    //
    // Create the query.
    //

    Query = EvtQuery(NULL, Channel, XPath, EvtQueryChannelPath);
    if (Query == NULL) {
        return GetLastError();
    }

    //
    // Read each event and render it as XML.
    //

    Buffer = NULL;
    BufferSize = 0;
    BufferSizeNeeded = 0;

    while (EvtNext(Query, 1, &Event, INFINITE, 0, &Count) != FALSE) {

        do {
            if (BufferSizeNeeded > BufferSize) {
                free(Buffer);
                BufferSize = BufferSizeNeeded;
                Buffer = malloc(BufferSize);
                if (Buffer == NULL) {
                    Status = ERROR_OUTOFMEMORY;
                    BufferSize = 0;
                    break;
                }
            }

            if (EvtRender(NULL,
                          Event,
                          EvtRenderEventXml,
                          BufferSize,
                          Buffer,
                          &BufferSizeNeeded,
                          &Count) != FALSE) {
                Status = ERROR_SUCCESS;
            } else {
                Status = GetLastError();
            }
        } while (Status == ERROR_INSUFFICIENT_BUFFER);

        //
        // Display either the event xml or an error message.
        //

        if (Status == ERROR_SUCCESS) {
            wprintf(L"%s\n", Buffer);
        } else {
            wprintf(L"Error rendering event.\n");
        }

        EvtClose(Event);
    }

    //
    // When EvtNextChannelPath returns ERROR_NO_MORE_ITEMS, we have actually
    // iterated through all matching events and thus succeeded.
    //
 
    Status = GetLastError();
    if (Status == ERROR_NO_MORE_ITEMS) {
        Status = ERROR_SUCCESS;
    }

    //
    // Free resources.
    //

    EvtClose(Query);
    free(Buffer);

    return Status;
}

ULONG
DisplayHelp (
    VOID
    )

/*++

Routine Description:

    This function prints to the standard output the the valid command-line
    arguments.

Arguments:

    None.

Return Value:

    ERROR_SUCCESS - this function always succeeds for now. It returns an
        error code for consistency with the other function called from wmain.

--*/

{
    wprintf(L"Usage: EventsQuery.exe [/? | Channel [xpath-filter]]\n");

    return ERROR_SUCCESS;
}

int __cdecl
wmain (
    __in int argc,
    __in_ecount(argc) WCHAR ** argv
    )

/*++

Routine Description:

    This function is the entry-point of this tool. It inspects the number of
    command-line arguments and call the appropriate worker routines.

Arguments:

    argc - Supplies the number of command-line options.

    argv - Supplies the parsed command-line options.

Return Value:

    Win32 error code indicating the status of the execution of the tool.

--*/

{
    ULONG Status;
    PCWSTR Xpath;

    Xpath = L"*";

    //
    // Call the appropriate worker routine based on the number of args.
    //

    switch (argc) {
    case 1:
        Status = EnumerateChannels();
        break;

    case 3:
        Xpath = argv[2];
        __fallthrough;

    case 2:
        if ((wcscmp(argv[1], L"/?") == 0) ||
            (wcscmp(argv[1], L"-?") == 0)) {
            Status = DisplayHelp();
        } else {
            Status = QueryEvents(argv[1], Xpath);
        }
        break;

    default:
        Status = DisplayHelp();
    }

    //
    // Let the user know if an error occurred.
    //

    if (Status != ERROR_SUCCESS) {
        wprintf(L"Error: %u\n", Status);
    }

    return Status;
}
