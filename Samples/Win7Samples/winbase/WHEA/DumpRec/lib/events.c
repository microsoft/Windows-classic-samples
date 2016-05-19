// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*++

Module Name:

    wheaevt.c

Abstract:

    This file contains routines used to extract WHEA error records from the WHEA
    event log channel.

--*/

#include "cperhlpp.h"

#define WHEA_CHANNEL        L"Microsoft-Windows-Kernel-WHEA/Errors"
#define WHEA_CHANNEL_LEGACY L"Microsoft-Windows-Kernel-WHEA"
#define WHEA_LOG_QUERY      L"*[System/Provider[@Name=\"Microsoft-Windows-Kernel-WHEA\"] and System/EventID=20]"

EVT_HANDLE
CperOpenWheaLogQuery (
    __in_opt PWSTR ComputerName,
    __in_opt PWSTR UserName,
    __in_opt PWSTR Domain,
    __in_opt PWSTR Password,
    __in_opt PWSTR FileName,
    __out EVT_HANDLE *Session
    )

/*++

Routine Description:

    This routine will initialize an event log query that may be used to
    enumerate any WHEA error records contained in the WHEA event log.

Arguments:

    ComputerName - Supplies an optional computer name for remote queries. This
        should be NULL for the local event log query or if an exported event log
        is to be queried.

    UserName - Supplies the username to be used to authenticate to the remote
        computer.

    Domain - Supplies the username to be used to authenticate to the remote
        computer.

    Password - Supplies the password to be used to authenticate to the remote
        computer.

    FileName - Supplies an optional filename for an exported event log. This
        should be NULL for a live (local or remote) event log query.

    Session - Supplies a variable in which a handle to the session is returned,
        but only if the query is for events on a remote computer.

Return Value:

    A handle to the ETW query if successful, NULL otherwise.

--*/

{

    DWORD Error;
    DWORD Flags;
    EVT_RPC_LOGIN Login;
    PCWSTR Path;
    EVT_HANDLE QueryHandle;
    EVT_HANDLE SessionHandle;

    QueryHandle = NULL;
    SessionHandle = NULL;
    Error = ERROR_SUCCESS;

    //
    // If a computer name is specified, then an event log session to that
    // computer must be opened. It is invalid to specify a remote computer as
    // well as a filename.
    //

    if (ComputerName != NULL) {
        if (FileName != NULL) {
            Error = ERROR_INVALID_PARAMETER;
            goto OpenWheaLogQueryEnd;
        }

        RtlZeroMemory(&Login, sizeof(EVT_RPC_LOGIN));
        Login.Server = ComputerName;
        Login.User = UserName;
        Login.Domain = Domain;
        Login.Password = Password;
        Login.Flags = EvtRpcLoginAuthDefault;
        SessionHandle = EvtOpenSession(EvtRpcLogin, &Login, 0, 0);
        if (SessionHandle == NULL) {
            Error = GetLastError();
            goto OpenWheaLogQueryEnd;
        }
    }

    if (FileName == NULL) {
        Path = WHEA_CHANNEL;
        Flags = EvtQueryChannelPath | EvtQueryForwardDirection;

    } else {
        Path = (PCWSTR)FileName;
        Flags = EvtQueryFilePath | EvtQueryForwardDirection;
    }

    //
    // Open the query. If this is not a file query and the open fails, try the
    // legacy log name.
    //

    QueryHandle = EvtQuery(SessionHandle, Path, WHEA_LOG_QUERY, Flags);
    if (QueryHandle == NULL) {
        Error = GetLastError();
        if (FileName == NULL) {
            Path = WHEA_CHANNEL_LEGACY;
            QueryHandle = EvtQuery(SessionHandle, Path, WHEA_LOG_QUERY, Flags);
            if (QueryHandle == NULL) {
                Error = GetLastError();
                goto OpenWheaLogQueryEnd;
            }
        }
    }

    *Session = SessionHandle;

OpenWheaLogQueryEnd:
    if (QueryHandle == NULL) {
        if (SessionHandle != NULL) {
            EvtClose(SessionHandle);
        }

        SetLastError(Error);
    }

    return QueryHandle;
}

__success(return != FALSE)
BOOL
CperGetNextWheaLogEntry (
    __in EVT_HANDLE QueryHandle,
    __out_bcount_part_opt(BufferSize, *ReturnedSize) PWHEA_ERROR_RECORD Record,
    __in DWORD BufferSize,
    __out DWORD *ReturnedSize
    )

/*++

Routine Description:

    This routine will return the next available WHEA error record logged to the
    WHEA error event log channel.

Arguments:

    QueryHandle - Supplies a handle to the event log query returned by a call to
        CperOpenWheaLogQuery.

    Buffer - Supplies a pointer to the buffer in which the error record is
        returned. This parameter may be NULL if the buffer size specified is
        zero. In this case, the function will return the required size in the
        returned size field.

    BufferSize - Supplies the length of the buffer in bytes.

    ReturnedSize - Supplies a pointer to a variable in which the length of the
        returned error record is returned. If the supplied buffer is too small
        then the required length is returned.

Return Value:

    TRUE upon success, FALSE otherwise. In the case of failure, the error code
    can be obtained by calling GetLastError(). If there are no error records
    remaining, the function will fail with the error code ERROR_NO_MORE_ITEMS.

--*/

{

    PVOID Buffer;
    PWHEA_ERROR_RECORD ErrorRecord;
    EVT_HANDLE EventHandle;
    DWORD LastError;
    DWORD PropertyCount;
    DWORD RecordSize;
    EVT_HANDLE RenderContext;
    BOOL Result;
    DWORD Returned;
    PEVT_VARIANT Variant;

    EventHandle = NULL;
    RenderContext = NULL;
    Buffer = NULL;
    LastError = NO_ERROR;

    Result = EvtNext(QueryHandle, 1, &EventHandle, (DWORD)-1, 0, &Returned);
    if (Result == FALSE) {
        EventHandle = NULL;
        goto GetNextWheaLogEntryEnd;
    }

    RenderContext = EvtCreateRenderContext(0, NULL, EvtRenderContextUser);
    if (RenderContext == NULL) {
        Result = FALSE;
        goto GetNextWheaLogEntryEnd;
    }

    //
    // Make the call to the rendering routine to determine the buffer size
    // required to contain the rendered event. This call should fail with an
    // insufficient buffer error. Anything else (including success) is an
    // immediate failure.
    //

    Result = EvtRender(RenderContext,
                       EventHandle,
                       EvtRenderEventValues,
                       0,
                       NULL,
                       &Returned,
                       &PropertyCount);

    if (Result != FALSE) {
        SetLastError(ERROR_GEN_FAILURE);
        Result = FALSE;
        goto GetNextWheaLogEntryEnd;
    }

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        goto GetNextWheaLogEntryEnd;
    }

    //
    // Allocate the buffer to contain the rendered event log entry and render
    // the entry.
    //

    Buffer = HeapAlloc(GetProcessHeap(), 0, Returned);
    if (Buffer == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        Result = FALSE;
        goto GetNextWheaLogEntryEnd;
    }

    Result = EvtRender(RenderContext,
                       EventHandle,
                       EvtRenderEventValues,
                       Returned,
                       Buffer,
                       &Returned,
                       &PropertyCount);

    if (Result == FALSE) {
        goto GetNextWheaLogEntryEnd;
    }

    //
    // This particular event has 2 properties. Anything less than this and
    // return failure.
    //

    if (PropertyCount < 2) {
        Result = FALSE;
        SetLastError(ERROR_GEN_FAILURE);
        goto GetNextWheaLogEntryEnd;
    }

    //
    // The first property is the error record length.
    //

    Variant = (PEVT_VARIANT)Buffer;
    if (Variant->Type != EvtVarTypeUInt32) {
        Result = FALSE;
        SetLastError(ERROR_GEN_FAILURE);
        goto GetNextWheaLogEntryEnd;
    }

    RecordSize = (DWORD)Variant->UInt32Val;

    //
    // The second property is the error record itself.
    //

    Variant += 1;
    if (Variant->Type != EvtVarTypeBinary) {
        Result = FALSE;
        SetLastError(ERROR_GEN_FAILURE);
        goto GetNextWheaLogEntryEnd;
    }

    ErrorRecord = (PWHEA_ERROR_RECORD)Variant->BinaryVal;

    //
    // Ensure that the record length property and the length field of the error
    // record header are the same.
    //

    if (ErrorRecord->Header.Length != RecordSize) {
        Result = FALSE;
        SetLastError(ERROR_GEN_FAILURE);
        goto GetNextWheaLogEntryEnd;
    }

    //
    // If the supplied buffer can contain the error record, then return it. If
    // not then return the appropriate error code as well as the buffer size
    // required to contain the error record. The query should also be
    // repositioned so that when the caller calls again to get the entry it will
    // retry the same one.
    //

    *ReturnedSize = RecordSize;
    if (BufferSize >= RecordSize) {
        RtlCopyMemory(Record, ErrorRecord, RecordSize);
        Result = TRUE;

    } else {
        EvtSeek(QueryHandle, -1, NULL, 0, EvtSeekRelativeToCurrent);
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        Result = FALSE;
    }

GetNextWheaLogEntryEnd:
    if (Result == FALSE) {
        LastError = GetLastError();
    }

    if (EventHandle != NULL) {
        EvtClose(EventHandle);
    }

    if (RenderContext != NULL) {
        EvtClose(RenderContext);
    }

    if (Buffer != NULL) {
        HeapFree(GetProcessHeap(), 0, Buffer);
    }

    if (Result == FALSE) {
        SetLastError(LastError);
    }

    return Result;
}

