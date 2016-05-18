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

    GetEventRawDescription.cpp

Abstract:

    This module contains the implementation of how to dump the raw event
    description without the substitution from payload.

Environment:

    User-mode only.

--*/

#include <windows.h>
#include <winevt.h>
#include <stdio.h>
#include <stdlib.h>

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
    wprintf(L"Usage: %s <ProviderName> [LocaleName]\n", ExeFile);
    wprintf(L"For Example: %s Microsoft-Windows-Eventlog en-us\n", ExeFile);
}

DWORD 
GetEventRawDescriptions (
    __in PCWSTR ProviderName,
    __in LCID Locale
    )

/*++

Routine Description:

    This function gets the raw event description strings.

Parameters:

    ProviderName - Supplies the provider name.

    Locale - Supplies the LCID.

Return Value:

    Win32 error code indicating the status of the function execution.

--*/

{
    PWSTR Description;
    ULONG BufferLength;
    ULONG BufferLengthNeeded;
    ULONG BufferUsed;
    EVT_VARIANT EventId;
    EVT_VARIANT EventMessageId;
    EVT_HANDLE EventMeta;
    EVT_HANDLE EventMetaEnum;
    EVT_HANDLE ProviderMetadata;
    ULONG Status;

    //
    // Open the provider meta data.
    //

    ProviderMetadata = EvtOpenPublisherMetadata(NULL,
                                                ProviderName,
                                                NULL,
                                                Locale,
                                                0);
    if (ProviderMetadata == NULL) {
        return GetLastError();
    }

    //
    // Open the Event meta data associated with the provider.
    //

    EventMetaEnum = EvtOpenEventMetadataEnum(ProviderMetadata, 0);
    if (EventMetaEnum == NULL) {
        Status = GetLastError();
        EvtClose(ProviderMetadata);
        return Status;
    }

    Description = NULL;
    BufferLength = 0;
    BufferLengthNeeded = 0;

    while ((EventMeta = EvtNextEventMetadata(EventMetaEnum, 0)) != NULL) {

        //
        // Get the event & message IDs.
        //

        if ((EvtGetEventMetadataProperty(EventMeta, 
                                         EventMetadataEventMessageID,
                                         0, 
                                         sizeof(EVT_VARIANT),
                                         &EventMessageId,
                                         &BufferUsed) == FALSE) ||
            (EvtGetEventMetadataProperty(EventMeta,
                                         EventMetadataEventID,
                                         0,
                                         sizeof(EVT_VARIANT),
                                         &EventId,
                                         &BufferUsed) == FALSE)) {
            EvtClose(EventMeta);
            continue;
        }

        //
        // Get the description, reallocating the buffer if needed.
        //

        do {
            if (BufferLengthNeeded > BufferLength) {
                free(Description);
                BufferLength = BufferLengthNeeded;
                Description = (PWSTR)malloc(BufferLength * sizeof(WCHAR));
                if (Description == NULL) {
                    Status = ERROR_OUTOFMEMORY;
                    BufferLength = 0;
                    break;
                }
            }

            if (EvtFormatMessage(ProviderMetadata,
                                 NULL,
                                 EventMessageId.UInt32Val,
                                 0,
                                 NULL,
                                 EvtFormatMessageId,
                                 BufferLength,
                                 Description,
                                 &BufferLengthNeeded) != FALSE) {
                Status = ERROR_SUCCESS;
            } else {
                Status = GetLastError();
            }
        } while (Status == ERROR_INSUFFICIENT_BUFFER);

        //
        // Display either the event message or an error message.
        //

        switch (Status) {
        case ERROR_SUCCESS:
        case ERROR_EVT_UNRESOLVED_VALUE_INSERT:
        case ERROR_EVT_UNRESOLVED_PARAMETER_INSERT:
        case ERROR_EVT_MAX_INSERTS_REACHED:
            wprintf(L"Event %u raw description is: %s\n",
                    EventId.UInt32Val,
                    Description);
            break;

        default:
            wprintf(L"Get raw event description failed with error code %u\n",
                    Status);
        }

        //
        // Close this event's metadata and go to the next one.
        //

        EvtClose(EventMeta);
    }

    Status = GetLastError();
    if (Status == ERROR_NO_MORE_ITEMS) {
        Status = ERROR_SUCCESS;
    }

    free(Description);
    EvtClose(EventMetaEnum);
    EvtClose(ProviderMetadata);

    return Status;
}

int __cdecl
wmain (
    __in int argc, 
    __in_ecount(argc) PWSTR* argv
)

/*++

Routine Description:

    This function is the entry-point of this tool.

Arguments:

    argc - Supplies the number of command-line options.

    argv - Supplies the parsed command-line options.

Return Value:

    Win32 error code indicating the status of the execution of the tool.

--*/

{
    LCID Locale;
    ULONG Status;

    if (argc < 2 || argc > 3) {
        ShowHelp(argv[0]);
        return ERROR_SUCCESS;
    }

    if (argc == 3) {
        Locale = LocaleNameToLCID(argv[2], 0);
    } else {
        Locale = 0;
    }

    Status = GetEventRawDescriptions(argv[1], Locale);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"Error: %u\n", Status);
    }

    return Status;
}
