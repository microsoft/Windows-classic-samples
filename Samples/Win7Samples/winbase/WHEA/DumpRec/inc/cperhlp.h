// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*++

Module Name:

    whearec.h

Abstract:

    This header declares the functions provided by the WHEA error record
    interpretation library.

--*/

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

EVT_HANDLE
CperOpenWheaLogQuery (
    __in_opt PWSTR ComputerName,
    __in_opt PWSTR UserName,
    __in_opt PWSTR Domain,
    __in_opt PWSTR Password,
    __in_opt PWSTR FileName,
    __out EVT_HANDLE *Session
    );

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

__success(return != FALSE)
BOOL
CperGetNextWheaLogEntry (
    __in EVT_HANDLE QueryHandle,
    __out_bcount_part_opt(BufferSize, *ReturnedSize) PWHEA_ERROR_RECORD Record,
    __in DWORD BufferSize,
    __out DWORD *ReturnedSize
    );

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


BOOL
CperIsValidErrorRecordSignature (
    __in PWHEA_ERROR_RECORD Record
    );

/*++

Routine Description:

    This routine will compare the error record signature with the proper values
    and return whether it is correct or not.

Arguments:

    Record - Supplies a pointer to the error record.

Return Value:

    TRUE if the error record signature is correct.

    FALSE otherwise.

--*/

BOOL
CperGetFirstSection (
    __in PWHEA_ERROR_RECORD Record,
    __out ULONG *Context,
    __out PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR *SectionDescriptor,
    __out_opt PVOID *SectionData
    );

/*++

Routine Description:

    This routine may be used to enumerate the sections in a common platform
    error record.

Arguments:

    Record - Supplies a pointer to the error record.

    Context - Supplies a pointer to a variable that maintains the current state
        of the search. The same variable should be used in subsequent calls to
        enumerate subsequent sections in the record.

    Descriptor - Supplies a location in which a pointer to the descriptor for
        the found section is returned.

    Section - Supplies an optional location in which a pointer to the found
        section is returned.

Return Value:

    TRUE upon success, FALSE otherwise. In the case of failure, the error code
    can be obtained by calling GetLastError(). If there are no error records
    remaining, the function will fail with the error code ERROR_NO_MORE_ITEMS.

--*/

BOOL
CperGetNextSection (
    __in PWHEA_ERROR_RECORD Record,
    __inout ULONG *Context,
    __out PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR *SectionDescriptor,
    __out_opt PVOID *SectionData
    );

/*++

Routine Description:

    This routine may be used to enumerate the sections in a common platform
    error record.

Arguments:

    Record - Supplies a pointer to the error record.

    Context - Supplies a pointer to a variable that maintains the current state
        of the search. The same variable should be used in subsequent calls to
        enumerate subsequent sections in the record.

    Descriptor - Supplies a location in which a pointer to the descriptor for
        the found section is returned.

    Section - Supplies an optional location in which a pointer to the found
        section is returned.

Return Value:

    TRUE upon success, FALSE otherwise. In the case of failure, the error code
    can be obtained by calling GetLastError(). If there are no error records
    remaining, the function will fail with the error code ERROR_NO_MORE_ITEMS.

--*/

BOOL
CperFindPrimarySection (
    __in PWHEA_ERROR_RECORD Record,
    __out PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR *SectionDescriptor,
    __out PVOID *SectionData
    );

/*++

Routine Description:

    This routine will find the primary section and return a reference to its
    descriptor and data. If no section is marked as primary, then by convention
    the first section (section 0) is returned as the primary section.

Arguments:

    Record - Supplies a pointer to the error record.

    SectionDescriptor - Supplies a buffer in which a pointer to the descriptor
        of the primary section will be returned.

    SectionData - Supplies a buffer in which a pointer to the body of the
        primary section will be returned.

Return Value:

    TRUE upon success, FALSE otherwise. In the case of failure, the error code
    can be obtained by calling GetLastError(). If there are no error records
    remaining, the function will fail with the error code ERROR_NO_MORE_ITEMS.

--*/


BOOL
CperFindSection (
    __in PWHEA_ERROR_RECORD Record,
    __in const GUID *SectionType,
    __out PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR *SectionDescriptor,
    __out PVOID *SectionData
    );

/*++

Routine Description:

    This routine provides a means to search an error record for a specific
    section.

Arguments:

    Record - Supplies a pointer to the error record.

    SectionType - Supplies a GUID specifying the section being sought. This may
        be any standard common platform error record or implementation specific
        section type.

    Descriptor - Supplies a location in which a pointer to the descriptor for
        the found section is returned.

    Section - Supplies an optional location in which a pointer to the found
        section is returned.

Return Value:

    If the function succeeds, the return value is nonzero (TRUE). If the
    function fails the return value is zero (FALSE). To get extended error
    information, call the GetLastError function.

--*/

#if defined(__cplusplus)
}
#endif

