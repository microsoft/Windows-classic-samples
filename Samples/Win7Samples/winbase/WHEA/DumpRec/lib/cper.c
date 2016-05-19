// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*++

Module Name:

    whearec.c

Abstract:

    This module provides the top level implementation of the functions provided
    by the WHEA record interpretation library.

--*/

#include "cperhlpp.h"

BOOL
CperIsValidErrorRecordSignature (
    __in PWHEA_ERROR_RECORD Record
    )

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

{

    if ((Record->Header.Signature != WHEA_ERROR_RECORD_SIGNATURE) ||
        (Record->Header.Revision.AsUSHORT > WHEA_ERROR_RECORD_REVISION) ||
        (Record->Header.SignatureEnd != WHEA_ERROR_RECORD_SIGNATURE_END)) {

        return FALSE;
    }

    return TRUE;
}

BOOL
CperGetFirstSection (
    __in PWHEA_ERROR_RECORD Record,
    __out ULONG *Context,
    __out PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR *SectionDescriptor,
    __out_opt PVOID *SectionData
    )

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

    If the function succeeds, the return value is nonzero (TRUE). If the
    function fails the return value is zero (FALSE). To get extended error
    information, call the GetLastError function.

--*/

{

    ULONG Next;
    BOOL Result;

    Next = 0;
    Result = CperGetNextSection(Record, &Next, SectionDescriptor, SectionData);
    if (Result != FALSE) {
        *Context = Next;
    }

    return Result;
}

BOOL
CperGetNextSection (
    __in PWHEA_ERROR_RECORD Record,
    __inout ULONG *Context,
    __out PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR *SectionDescriptor,
    __out_opt PVOID *SectionData
    )

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

    If the function succeeds, the return value is nonzero (TRUE). If the
    function fails the return value is zero (FALSE). To get extended error
    information, call the GetLastError function.

--*/

{

    PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR Descriptor;
    ULONG Error;
    ULONG Index;
    ULONG MinimumLength;
    BOOL Result;

    Result = TRUE;
    Error = NO_ERROR;
    if ((Record == NULL) ||
        (Context == NULL) ||
        (SectionDescriptor == NULL) ||
        (CperIsValidErrorRecordSignature(Record) == FALSE) ||
        (Record->Header.SectionCount == 0)) {

        Result = FALSE;
        Error = ERROR_INVALID_PARAMETER;
        goto GetNextSectionEnd;
    }

    //
    // Ensure that the supplied record is at least as long as required to store
    // the descriptors for the sections supposedly in the record.
    //

    MinimumLength = sizeof(WHEA_ERROR_RECORD_HEADER) +
        (Record->Header.SectionCount *
         sizeof(WHEA_ERROR_RECORD_SECTION_DESCRIPTOR));

    if (Record->Header.Length < MinimumLength) {
        Result = FALSE;
        Error = ERROR_INVALID_PARAMETER;
        goto GetNextSectionEnd;
    }

    //
    // If the index is greater than the number of sections, then it has been
    // incorrectly fabricated by the caller or the record had section removed
    // during the enumeration. Either way, this is different to the case where
    // there are no sections left.
    //

    Index = *Context;
    if (Index > Record->Header.SectionCount) {
        Result = FALSE;
        Error = ERROR_INVALID_PARAMETER;
        goto GetNextSectionEnd;
    }

    if (Index == Record->Header.SectionCount) {
        Result = FALSE;
        Error = ERROR_NOT_FOUND;
        goto GetNextSectionEnd;
    }

    Descriptor = &Record->SectionDescriptor[Index];

    //
    // If the descriptor describes a section that is not completely contained
    // within the record then the record is invalid.
    //

    if ((Descriptor->SectionOffset + Descriptor->SectionLength) >
        Record->Header.Length) {

        Result = FALSE;
        Error = ERROR_INVALID_PARAMETER;
        goto GetNextSectionEnd;
    }

    *Context = Index + 1;
    *SectionDescriptor = Descriptor;
    if (SectionData != NULL) {
        *SectionData = (PVOID)(((PUCHAR)Record) + Descriptor->SectionOffset);
    }

GetNextSectionEnd:
    if (Result == FALSE) {
        SetLastError(Error);
    }

    return Result;
}

BOOL
CperFindPrimarySection (
    __in PWHEA_ERROR_RECORD Record,
    __out PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR *SectionDescriptor,
    __out PVOID *SectionData
    )

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

{

    ULONG Context;
    PVOID Data;
    PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR Descriptor;
    BOOL Result;

    Result = CperGetFirstSection(Record,
                                 &Context,
                                 &Descriptor,
                                 &Data);

    while (Result != FALSE) {
        if (Descriptor->Flags.Primary == 1) {
            *SectionDescriptor = Descriptor;
            *SectionData = Data;
            break;
        }

        Result = CperGetNextSection(Record,
                                     &Context,
                                     &Descriptor,
                                     &Data);
    }

    //
    // If no primary section was found, then get the first section and return
    // that.
    //

    if ((Result == FALSE) && (GetLastError() == ERROR_NOT_FOUND)) {
        Result = CperGetFirstSection(Record,
                                     &Context,
                                     SectionDescriptor,
                                     SectionData);
    }

    return Result;
}

BOOL
CperFindSection (
    __in PWHEA_ERROR_RECORD Record,
    __in const GUID *SectionType,
    __out PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR *SectionDescriptor,
    __out PVOID *SectionData
    )

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

{

    ULONG Context;
    PVOID Data;
    PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR Descriptor;
    BOOL Result;

    Result = CperGetFirstSection(Record,
                                 &Context,
                                 &Descriptor,
                                 &Data);

    while (Result != FALSE) {
        if (IsEqualGUID(&Descriptor->SectionType, SectionType)) {
            break;
        }

        Result = CperGetNextSection(Record,
                                    &Context,
                                    &Descriptor,
                                    &Data);
    }

    if (Result != FALSE) {
        *SectionDescriptor = Descriptor;
        *SectionData = Data;
    }

    return Result;
}

