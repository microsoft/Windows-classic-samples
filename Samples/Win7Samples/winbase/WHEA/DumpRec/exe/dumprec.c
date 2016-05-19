// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*++

Module Name:

    dumprec.c

Abstract:

    This is the main module for the WHEA DUMPREC application.

--*/

#include "dumprec.h"
#include <initguid.h>
#include <cperguid.h>

#define BACK_SPACE                      0x08
#define CARRIAGE_RETURN                 0x0D
#define MAXIMUM_PASSWORD_LENGTH         (RTL_NUMBER_OF(DrPasswordBuffer) - 1)

typedef struct _GUID_LOOKUP_ENTRY {
    const GUID * Guid;
    UINT StringId;
};

const struct _GUID_LOOKUP_ENTRY GuidLookup[] = {
    { &CMC_NOTIFY_TYPE_GUID, IDS_WHEA_GUID_CMC_NOTIFY_TYPE },
    { &CPE_NOTIFY_TYPE_GUID, IDS_WHEA_GUID_CPE_NOTIFY_TYPE },
    { &MCE_NOTIFY_TYPE_GUID, IDS_WHEA_GUID_MCE_NOTIFY_TYPE },
    { &PCIe_NOTIFY_TYPE_GUID, IDS_WHEA_GUID_PCIE_NOTIFY_TYPE },
    { &INIT_NOTIFY_TYPE_GUID, IDS_WHEA_GUID_INIT_NOTIFY_TYPE },
    { &NMI_NOTIFY_TYPE_GUID, IDS_WHEA_GUID_NMI_NOTIFY_TYPE },
    { &BOOT_NOTIFY_TYPE_GUID, IDS_WHEA_GUID_BOOT_NOTIFY_TYPE },
    { &PROCESSOR_GENERIC_ERROR_SECTION_GUID, IDS_WHEA_GUID_PROCESSOR_GENERIC_ERROR_SECTION },
    { &XPF_PROCESSOR_ERROR_SECTION_GUID, IDS_WHEA_GUID_XPF_PROCESSOR_ERROR_SECTION },
    { &IPF_PROCESSOR_ERROR_SECTION_GUID, IDS_WHEA_GUID_IPF_PROCESSOR_ERROR_SECTION },
    { &MEMORY_ERROR_SECTION_GUID, IDS_WHEA_GUID_MEMORY_ERROR_SECTION },
    { &PCIEXPRESS_ERROR_SECTION_GUID, IDS_WHEA_GUID_PCIEXPRESS_ERROR_SECTION },
    { &PCIXBUS_ERROR_SECTION_GUID, IDS_WHEA_GUID_PCIXBUS_ERROR_SECTION },
    { &PCIXDEVICE_ERROR_SECTION_GUID, IDS_WHEA_GUID_PCIXDEVICE_ERROR_SECTION },
    { &FIRMWARE_ERROR_RECORD_REFERENCE_GUID, IDS_WHEA_GUID_FIRMWARE_ERROR_RECORD_REFERENCE_SECTION },
    { &WHEA_CACHECHECK_GUID, IDS_WHEA_GUID_CACHE_CHECK },
    { &WHEA_TLBCHECK_GUID, IDS_WHEA_GUID_TLB_CHECK },
    { &WHEA_BUSCHECK_GUID, IDS_WHEA_GUID_BUS_CHECK },
    { &WHEA_MSCHECK_GUID, IDS_WHEA_GUID_MS_CHECK },
    { &WHEA_RECORD_CREATOR_GUID, IDS_WHEA_GUID_WHEA_RECORD_CREATOR },
    { &GENERIC_NOTIFY_TYPE_GUID, IDS_WHEA_GUID_GENERIC_NOTIFY_TYPE },
    { &IPF_SAL_RECORD_SECTION_GUID, IDS_WHEA_GUID_IPF_SAL_RECORD_SECTION },
    { &XPF_MCA_SECTION_GUID, IDS_WHEA_GUID_XPF_MCA_SECTION },
    { &NMI_SECTION_GUID, IDS_WHEA_GUID_NMI_SECTION },
    { &GENERIC_SECTION_GUID, IDS_WHEA_GUID_GENERIC_SECTION },
    { &WHEA_ERROR_PACKET_SECTION_GUID, IDS_WHEA_GUID_ERROR_PACKET_SECTION }
};

PWSTR DrComputerName = NULL;
PWSTR DrUserName = NULL;
PWSTR DrDomain = NULL;
PWSTR DrPassword = NULL;
PWSTR DrFileName = NULL;
WCHAR DrPasswordBuffer[256] = {0};

BOOL
DrParseCommandLine (
    __in int argc,
    __in_ecount(argc) PWSTR argv[]
    );

BOOL
DrGetPassword (
    VOID
    );

VOID
DrUsage(
    VOID
    );

BOOL
DrDumpRecords (
    VOID
    );

BOOL
DrValidateRecord (
    __in_bcount(Size) PWHEA_ERROR_RECORD Record,
    __in ULONG Size
    );

VOID
DrDumpRecordHeader (
    __in_bcount(Size) PWHEA_ERROR_RECORD Record,
    __in ULONG Size
    );

VOID
DrDumpRecordSections (
    __in_bcount(Size) PWHEA_ERROR_RECORD Record,
    __in ULONG Size
    );

VOID
DrDumpSectionDescriptor (
    __in DWORD Index,
    __in PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR Descriptor
    );

VOID
DrDumpProcessorGenericErrorSection (
    __in PWHEA_PROCESSOR_GENERIC_ERROR_SECTION Data
    );

VOID
DrDumpMemoryErrorSection (
    __in PWHEA_MEMORY_ERROR_SECTION Data
    );

VOID
DrDumpXpfMcaSection (
    __in PWHEA_XPF_MCA_SECTION Data
    );

PWSTR
DrWheaGuid (
    __in const GUID *Guid,
    __out_ecount(BufferLength) PWSTR Buffer,
    __in DWORD BufferLength
    );

PWSTR
DrWheaRecordFlagsString (
    __in const WHEA_ERROR_RECORD_HEADER_FLAGS Flags,
    __out_ecount(BufferLength) PWSTR Buffer,
    __in DWORD BufferLength
    );

PCWSTR
DrWheaSeverityString(
    __in WHEA_ERROR_SEVERITY Severity
    );

PWSTR
DrWheaTime (
    __in const WHEA_TIMESTAMP *Timestamp,
    __out_ecount(BufferLength) PWSTR Buffer,
    __in DWORD BufferLength
    );

VOID
DrRcPrint (
    __in UINT FormatId,
    ...
    );

PWSTR
DrBitmap (
    __in ULONGLONG Value,
    __in ULONG Bits,
    __out_ecount(BufferLength) PWSTR Buffer,
    __in DWORD BufferLength
    );

int
__cdecl
wmain (
    __in int argc,
    __in_ecount(argc) PWSTR argv[]
    )

/*++

Routine Description:

    This is the main routine for the dumprec tool.

Arguments:

    args - Supplies the count of command-line arguments.

    argv - Supplies an array of strings that constitute the command-line
        arguments.

Return Value:

    Zero upon success, 255 otherwise.

--*/

{

    BOOL Result;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    Result = DrParseCommandLine(argc, argv);
    if (Result == FALSE) {
        DrUsage();
        return 255;
    }

    Result = DrDumpRecords();
    if (Result == FALSE) {
        return 255;
    }

    return 0;
}

BOOL
DrParseCommandLine (
    __in int argc,
    __in_ecount(argc) PWSTR argv[]
    )

/*++

Routine Description:

    This routine parses the application command line.

Arguments:

    argc - Supplies the number of arguments.

    argv - Supplies the array of arguments.

Return Value:

    TRUE if successful, FALSE otherwise.

--*/

{

    BOOL Error;
    int Count;
    PWSTR Parameter;
    PWSTR ComputerName;
    PWSTR UserName;
    PWSTR Password;
    PWSTR FileName;

    ComputerName = NULL;
    UserName = NULL;
    Password = NULL;
    FileName = NULL;
    Error = FALSE;
    for (Count = 1; Count < argc; Count += 1) {
        Parameter = argv[Count];
        if ((_wcsicmp(Parameter, L"/m") == 0) ||
            (_wcsicmp(Parameter, L"-m") == 0)) {

            Count += 1;
            if ((ComputerName != NULL) || (Count >= argc)) {
                Error = TRUE;
                break;
            }

            ComputerName = argv[Count];

        } else if ((_wcsicmp(Parameter, L"/u") == 0) ||
                   (_wcsicmp(Parameter, L"-u") == 0)) {

            Count += 1;
            if ((UserName != NULL) || (Count >= argc)) {
                Error = TRUE;
                break;
            }

            UserName = argv[Count];
            Count += 1;
            if ((Password != NULL) || (Count >= argc)) {
                Error = TRUE;
                break;
            }

            Password = argv[Count];

        } else if ((*Parameter == L'/') ||
                   (*Parameter == L'-')) {

            Error = TRUE;
            break;

        } else {
            if (FileName != NULL) {
                Error = TRUE;
                break;
            }

            FileName = argv[Count];
        }
    }

    if (Error != FALSE) {
        return FALSE;
    }

    if ((UserName != NULL) && (ComputerName == NULL)) {
        return FALSE;
    }

    if ((FileName != NULL) && ((ComputerName != NULL) || (UserName != NULL))) {
        return FALSE;
    }

    DrComputerName = ComputerName;

    //
    // If the username is is of the form domain\user then split the components
    // so that they can be passed to the event query.
    //

    if (UserName != NULL) {
        DrUserName = wcschr(UserName, L'\\');
        if (DrUserName == NULL) {
            DrUserName = UserName;

        } else {
            DrDomain = UserName;
            *DrUserName++ = UNICODE_NULL;
        }
    }

    //
    // If the password was supplied as '*' on the command line, then use the
    // interactive password entry routine to obtain the password from the user.
    //

    if (Password != NULL) {
        if (wcscmp(Password, L"*") == 0) {
            DrPassword = DrPasswordBuffer;
            wprintf(L"Password: ");
            if (!DrGetPassword()) {
                wprintf(L"\n");
                return FALSE;
            }

            wprintf(L"\n");

        } else {
            DrPassword = Password;
        }
    }

    DrFileName = FileName;
    return TRUE;
}

VOID
DrUsage(
    VOID
    )

/*++

Routine Description:

    This routine displays the program usage to the user. This is done when the
    user provides invalid command line usage or the help prompt.

Arguments:

    None.

Return Value:

    None.

--*/

{

    wprintf(L"DUMPREC: WHEA event log dump\n\n"
            L"dumprec <filename>\n"
            L"dumprec [/m <computername> [/u <username> <password>|*]]\n\n"
            L"<filename> is an exported event log.\n"
            L"<computername> is the name of a remote computer.\n"
            L"<username> and <password> are remote computer credentials.\n");
}


BOOL
DrGetPassword (
    VOID
    )

/*++

Routine Description:

    This routine will obtain a password from the user, and place it in the
    global password buffer.

Arguments:

    None.

Return Value:

    TRUE if successful, FALSE otherwise.

--*/

{

    DWORD CharactersRead;
    DWORD Index;
    WCHAR Input;
    HANDLE InputConsole;
    DWORD PreviousMode;
    BOOL Redirected;
    BOOL Result;
    BOOL ConsoleModeSet;

    Index = 0;
    PreviousMode = 0;
    Redirected = FALSE;
    ConsoleModeSet = FALSE;

    InputConsole = GetStdHandle(STD_INPUT_HANDLE);
    if (InputConsole == NULL) {
        Result = FALSE;
        goto GetPasswordEnd;
    }

    //
    // Determine whether standard input has been redirected.
    //

    if ((InputConsole != (HANDLE)(ULONG_PTR)0x0000000F) &&
        (InputConsole != (HANDLE)(ULONG_PTR)0x00000003) &&
        (InputConsole != INVALID_HANDLE_VALUE)) {

        Redirected = TRUE;
    }

    //
    // Only set the console mode if input has not been redirected.
    //

    if (Redirected  == FALSE) {
        GetConsoleMode(InputConsole, &PreviousMode);
        Result = SetConsoleMode(InputConsole, ENABLE_PROCESSED_INPUT);
        if (Result == FALSE) {
            goto GetPasswordEnd;
        }

        ConsoleModeSet = TRUE;
    }

    //
    // Read input characters one at a time. If input is redirected, then use
    // file I/O. Handle backspace and carriage return keystrokes.
    //

    for (;;) {
        if (Redirected != FALSE) {
            Result = ReadFile(InputConsole,
                              &Input,
                              1,
                              &CharactersRead,
                              NULL);

            if (Result == FALSE) {
                goto GetPasswordEnd;
            }

            if (CharactersRead == 0) {
                break;
            }

        } else {
            Result = ReadConsole(InputConsole,
                                 &Input,
                                 1,
                                 &CharactersRead,
                                 NULL);

            if (Result == FALSE) {
                goto GetPasswordEnd;
            }
        }

        if (Input == CARRIAGE_RETURN) {
            break;
        }

        if (Input == BACK_SPACE) {
            if (Index != 0) {
                Index -= 1;
            }

            continue;
        }

        if (Index < MAXIMUM_PASSWORD_LENGTH) {
            if (Input != L'\n') {
                DrPasswordBuffer[Index] = Input;
                Index += 1;
            }
        }
    }

    //
    // Null terminate the password string, and return success.
    //

    DrPasswordBuffer[Index] = UNICODE_NULL;
    Result = TRUE;

GetPasswordEnd:

    //
    // For security reasons, the password buffer should be erased as soon as
    // possible.
    //

    if (Result == FALSE) {
        RtlZeroMemory(DrPasswordBuffer, sizeof(DrPasswordBuffer));
    }

    if (ConsoleModeSet != FALSE) {
        SetConsoleMode(InputConsole, PreviousMode);
    }

    return Result;
}

BOOL
DrDumpRecords (
    VOID
    )

/*++

Routine Description:

    This routine will open the WHEA error log channel and enumerate the WHEA
    error events. The contents of the WHEA error record will be displayed to
    the console.

Arguments:

    None.

Return Value:

    TRUE upon success, FALSE otherwise.

--*/

{

    DWORD BufferSize;
    DWORD ErrorCode;
    EVT_HANDLE QueryHandle;
    BOOL Reallocate;
    PWHEA_ERROR_RECORD Record;
    BOOL Result;
    EVT_HANDLE SessionHandle;
    DWORD Size;

    ErrorCode = NO_ERROR;
    Record = NULL;
    BufferSize = 0;
    Reallocate = FALSE;
    SessionHandle = NULL;

    //
    // Initialize the event log channel query, and then erase the password from
    // the password buffer.
    //

    QueryHandle = CperOpenWheaLogQuery(DrComputerName,
                                       DrUserName,
                                       DrDomain,
                                       DrPassword,
                                       DrFileName,
                                       &SessionHandle);

    RtlZeroMemory(DrPasswordBuffer, sizeof(DrPasswordBuffer));
    if (QueryHandle == NULL) {
        Result = FALSE;
        goto DumpRecordsEnd;
    }

    //
    // Iterate through the entries in the WHEA event log. The buffer used to
    // hold the error record should be grown if necessary, but only a maximum of
    // once per log entry.
    //

    for (;;) {
        Result = CperGetNextWheaLogEntry(QueryHandle,
                                         Record,
                                         BufferSize,
                                         &Size);

        if (Result == FALSE) {
            ErrorCode = GetLastError();
            if (ErrorCode == ERROR_NO_MORE_ITEMS) {
                Result = TRUE;
            }

            if ((ErrorCode != ERROR_INSUFFICIENT_BUFFER) ||
                (Reallocate != FALSE)) {

                break;
            }
        }

        if (Size > BufferSize) {
            Reallocate = TRUE;
            if (Record != NULL) {
                HeapFree(GetProcessHeap(), 0, Record);
            }

            Record = HeapAlloc(GetProcessHeap(), 0, Size);
            if (Record == NULL) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                Result = FALSE;
                break;
            }

            BufferSize = Size;
            continue;
        }

        Reallocate = FALSE;

        //
        // Dump the contents of the error record.
        //

        DrDumpRecordHeader(Record, Size);
        DrDumpRecordSections(Record, Size);

        //
        // Insert a blank line between error records.
        //

        wprintf(L"\n");
    }

DumpRecordsEnd:

    //
    // Preserve the error code while cleaning up.
    //

    if (Result == FALSE) {
        ErrorCode = GetLastError();
    }

    if (QueryHandle != NULL) {
        EvtClose(QueryHandle);
    }

    if (SessionHandle != NULL) {
        EvtClose(SessionHandle);
    }

    if (Record != NULL) {
        HeapFree(GetProcessHeap(), 0, Record);
    }

    if (Result == FALSE) {
        SetLastError(ErrorCode);
    }

    return Result;
}

BOOL
DrValidateRecord (
    __in_bcount(Size) PWHEA_ERROR_RECORD Record,
    __in ULONG Size
    )

/*++

Routine Description:

    This routine does some validation of an error record. It ensures that it is
    large enough to contain all of the error record descriptors, as well as the
    sections described by each descriptor.  The error record signature is also
    validated.

Arguments:

    Record - Supplies the error record.

    Size - Supplies the size of the buffer containing the error record.

Return Value:

    TRUE if the error record appears to be well-formed, FALSE otherwise.

--*/

{

    ULONG Count;
    PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR Descriptor;
    ULONG Length;
    ULONG RequiredLength;
    ULONG SectionCount;
    ULONG SectionStart;
    ULONG SectionEnd;
    BOOL Valid;

    Valid = TRUE;
    if ((Size < sizeof(WHEA_ERROR_RECORD_HEADER)) ||
        (Record->Header.Length > Size) ||
        (Record->Header.Signature != WHEA_ERROR_RECORD_SIGNATURE) ||
        (Record->Header.Revision.AsUSHORT < WHEA_ERROR_RECORD_REVISION) ||
        (Record->Header.SignatureEnd != WHEA_ERROR_RECORD_SIGNATURE_END)) {

        Valid = FALSE;
        goto ValidateRecordEnd;
    }

    Length = Record->Header.Length;
    SectionCount = Record->Header.SectionCount;
    RequiredLength = sizeof(WHEA_ERROR_RECORD) +
        (sizeof(WHEA_ERROR_RECORD_SECTION_DESCRIPTOR) * SectionCount);

    if (RequiredLength < Length) {
        Valid = FALSE;
        goto ValidateRecordEnd;
    }

    for (Count = 0; Count < SectionCount; Count += 1) {
        Descriptor = &Record->SectionDescriptor[Count];
        SectionStart = Descriptor->SectionOffset;
        SectionEnd = SectionStart + Descriptor->SectionLength;
        if ((SectionStart >= Length) || (SectionEnd >= Length)) {
            Valid = FALSE;
            goto ValidateRecordEnd;
        }
    }

ValidateRecordEnd:
    if (Valid == FALSE) {
        wprintf(L"============================================================\n");
        DrRcPrint(IDS_LABEL_INVALID_ERROR_RECORD);
    }

    return Valid;
}


VOID
DrDumpRecordHeader (
    __in_bcount(Size) PWHEA_ERROR_RECORD Record,
    __in ULONG Size
    )

/*++

Routine Description:

    This routine dumps the contents of the specified record to the console.

Arguments:

    Record - Supplies a pointer to the error record.

    Size - Supplies the size of the buffer containing the record.

Return Value:

    VOID

--*/

{

    WCHAR Buffer[80];
    ULONG Length;
    ULONG Indent;

    UNREFERENCED_PARAMETER(Size);

    Length = Record->Header.Length;

    __analysis_assume(Length <= Size);
    __analysis_assume(Length >= sizeof(WHEA_ERROR_RECORD_HEADER));

    wprintf(L"============================================================\n");
    DrWheaTime(&Record->Header.Timestamp, Buffer, RTL_NUMBER_OF(Buffer));
    wprintf(L"%s - ", Buffer);
    Indent = (ULONG)wcslen(Buffer) + 3;
    DrWheaGuid(&Record->Header.NotifyType, Buffer, RTL_NUMBER_OF(Buffer));
    wprintf(L"%s\n", Buffer);

    //
    // Error record header severity. If there are flags associated with this
    // record, then display them in parentheses after the severity.
    //

    wprintf(L"%*s%s",
            Indent,
            L"",
            DrWheaSeverityString(Record->Header.Severity));

    if (Record->Header.Flags.AsULONG == 0) {
        wprintf(L"\n");

    } else {
        DrWheaRecordFlagsString(Record->Header.Flags,
                                Buffer,
                                RTL_NUMBER_OF(Buffer));

        wprintf(L" (%s)\n", Buffer);
    }

    //
    // Platform identifier.
    //

    if (Record->Header.ValidBits.PlatformId == 1) {
        DrWheaGuid(&Record->Header.PlatformId, Buffer, RTL_NUMBER_OF(Buffer));
        DrRcPrint(IDS_LABEL_PLATFORM_ID, Buffer);
    }

    //
    // Partition identifier.
    //

    if (Record->Header.ValidBits.PartitionId == 1) {
        DrWheaGuid(&Record->Header.PartitionId, Buffer, RTL_NUMBER_OF(Buffer));
        DrRcPrint(IDS_LABEL_PARTITION_ID, Buffer);
    }

    wprintf(L"------------------------------------------------------------\n");
    return;
}

VOID
DrDumpRecordSections (
    __in_bcount(Size) PWHEA_ERROR_RECORD Record,
    __in ULONG Size
    )

/*++

Routine Description:

    This function iterates through the sections in the error record, dumping the
    contents.

Arguments:

    Record - Supplies a pointer to the error record.

Return Value:

    None.

--*/

{

    ULONG Context;
    ULONG Index;
    PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR Descriptor;
    PVOID Data;
    BOOL Result;

    UNREFERENCED_PARAMETER(Size);

    Index = 0;
    Result = CperGetFirstSection(Record, &Context, &Descriptor, &Data);
    while (Result != FALSE) {
        DrDumpSectionDescriptor(Index, Descriptor);
        if (IsEqualGUID(&Descriptor->SectionType,
                        &PROCESSOR_GENERIC_ERROR_SECTION_GUID)) {


            DrDumpProcessorGenericErrorSection(Data);

        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &XPF_PROCESSOR_ERROR_SECTION_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &IPF_PROCESSOR_ERROR_SECTION_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &MEMORY_ERROR_SECTION_GUID)) {

            DrDumpMemoryErrorSection(Data);


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &PCIEXPRESS_ERROR_SECTION_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &PCIXBUS_ERROR_SECTION_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &PCIXDEVICE_ERROR_SECTION_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &FIRMWARE_ERROR_RECORD_REFERENCE_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &IPF_SAL_RECORD_SECTION_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &XPF_MCA_SECTION_GUID)) {

            DrDumpXpfMcaSection(Data);

        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &NMI_SECTION_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &GENERIC_SECTION_GUID)) {


        } else if (IsEqualGUID(&Descriptor->SectionType,
                               &WHEA_ERROR_PACKET_SECTION_GUID)) {


        }














        Index += 1;
        Result = CperGetNextSection(Record, &Context, &Descriptor, &Data);
    }
}

VOID
DrDumpSectionDescriptor (
    __in DWORD Index,
    __in PWHEA_ERROR_RECORD_SECTION_DESCRIPTOR Descriptor
    )

/*++

Routine Description:

    This routine will display the specified record section descriptor .

Arguments:

    Index - Supplies the ordinal index of the section.

    Descriptor - Supplies a pointer to the descriptor.

Return Value:

    None.

--*/

{

    WCHAR Buffer[80];

    wprintf(L"%3u - %s",
            Index,
            DrWheaGuid(&Descriptor->SectionType,
                       Buffer,
                       RTL_NUMBER_OF(Buffer)));

    if (Descriptor->Flags.AsULONG != 0) {
        Buffer[0] = L'\0';
        if (Descriptor->Flags.Primary != 0) {
            wcscat_s(Buffer, 80, L" Primary");
        }

        if (Descriptor->Flags.ContainmentWarning != 0) {
            wcscat_s(Buffer, 80, L" ContainmentWarning");
        }

        if (Descriptor->Flags.Reset != 0) {
            wcscat_s(Buffer, 80, L" Reset");
        }

        if (Descriptor->Flags.ThresholdExceeded != 0) {
            wcscat_s(Buffer, 80, L" ThresholdExceeded");
        }

        if (Descriptor->Flags.ResourceNotAvailable != 0) {
            wcscat_s(Buffer, 80, L" ResourceNotAvailable");
        }

        if (Descriptor->Flags.LatentError != 0) {
            wcscat_s(Buffer, 80, L" LatentError");
        }

        if (Descriptor->Flags.Reserved != 0) {
            wcscat_s(Buffer, 80, L" InvalidFlags");
        }

        if (wcslen(Buffer) > 0) {
            wprintf(L" (%s)\n", &Buffer[1]);
        }

    } else {
        wprintf(L"\n");
    }

    if (Descriptor->ValidBits.FRUId != 0) {
        wprintf(L"      FRU ID:   %s\n",
                DrWheaGuid(&Descriptor->FRUId,
                           Buffer,
                           RTL_NUMBER_OF(Buffer)));
    }

    if (Descriptor->ValidBits.FRUText != 0) {
        wprintf(L"      FRU Text: %.20S\n", &Descriptor->FRUText[0]);
    }
}

PCWSTR
DrWheaSeverityString(
    __in WHEA_ERROR_SEVERITY Severity
    )

/*++

Routine Description:

    This routine will return a localized string representation of the specified
    severity value.

    N.B. The insertion strings in the resources must contain explicit NULL
         termination to allow them to be use in-place.

Arguments:

    Severity - Supplies the error severity to be displayed.

Return Value:

    None.

--*/

{

    PCWSTR String;
    UINT IdsMessage;
    int Return;

    //
    // If the severity value is not one of the well-known values then retrieve
    // the "unknown" string.
    //

    if (Severity <= IDS_WHEA_SEVERITY_MAX) {
        IdsMessage = (UINT)(IDS_WHEA_SEVERITY_BASE + Severity);

    } else {
        IdsMessage = IDS_WHEA_UNKNOWN;
    }

    //
    // The insertion strings in the resources must contain explicit NULL
    // termination to allow them to be used in-place.
    //

    Return = LoadString(GetModuleHandle(NULL), IdsMessage, (PWSTR)&String, 0);
    if (Return == 0) {
        String = L"Unknown";
    }

    return String;
}

VOID
DrDumpProcessorGenericErrorSection (
    __in PWHEA_PROCESSOR_GENERIC_ERROR_SECTION Data
    )

/*++

Routine Description:

    This routine dumps the contents of the processor generic error section.

Arguments:

    Data - Supplies a pointer to the section data.

Return Value:

    None.

--*/

{

    const WCHAR *ProcessorTypes[] = {
        L"x86/x64", L"Itanium"
    };

    const WCHAR *InstructionSets[] = {
        L"x86", L"ia64", L"x64"
    };

    const WCHAR *OperationType[] = {
        L"Generic", L"Data Read", L"Data Write", L"Instruction Execution"
    };

    if (Data->ValidBits.ProcessorType != 0) {
        wprintf(L"      Processor type:   %s\n",
                Data->ProcessorType < RTL_NUMBER_OF(ProcessorTypes) ?
                ProcessorTypes[Data->ProcessorType] : L"Invalid");
    }

    if (Data->ValidBits.InstructionSet != 0) {
        wprintf(L"      Instruction set:  %s\n",
                Data->ProcessorType < RTL_NUMBER_OF(InstructionSets) ?
                InstructionSets[Data->InstructionSet] : L"Invalid");
    }

    if (Data->ValidBits.ErrorType != 0) {
        wprintf(L"      Error type:       %s\n",
                Data->ErrorType & GENPROC_PROCERRTYPE_CACHE ? L"Cache" :
                Data->ErrorType & GENPROC_PROCERRTYPE_TLB ? L"TLB" :
                Data->ErrorType & GENPROC_PROCERRTYPE_BUS ? L"BUS" :
                Data->ErrorType & GENPROC_PROCERRTYPE_MAE ? L"MAE" :
                L"Unknown");
    }

    if (Data->ValidBits.Operation != 0) {
        wprintf(L"      Operation:        %s\n",
                Data->Operation < RTL_NUMBER_OF(OperationType) ?
                OperationType[Data->Operation] : L"Invalid");
    }

    if (Data->ValidBits.Flags != 0) {
        wprintf(L"      Flags:           %s%s%s%s\n",
                Data->Flags & GENPROC_FLAGS_RESTARTABLE ? L" Restartable" : L"",
                Data->Flags & GENPROC_FLAGS_PRECISEIP ? L" PreciseIp" : L"",
                Data->Flags & GENPROC_FLAGS_OVERFLOW ? L" Overflow" : L"",
                Data->Flags & GENPROC_FLAGS_CORRECTED ? L" Corrected" : L"");
    }

    if (Data->ValidBits.Level != 0) {
        wprintf(L"      Level:            %u\n", Data->Level);
    }

    if (Data->ValidBits.CPUVersion != 0) {
        wprintf(L"      CPU Version:      0x%016I64x\n", Data->CPUVersion);
    }

    if (Data->ValidBits.CPUBrandString != 0) {
        wprintf(L"      CPU Brand String: %.128S\n", Data->CPUBrandString);
    }

    if (Data->ValidBits.ProcessorId != 0) {
        wprintf(L"      Processor ID:     0x%I64x\n", Data->ProcessorId);
    }

    if (Data->ValidBits.TargetAddress != 0) {
        wprintf(L"      Target Address:   0x%016I64x\n", Data->TargetAddress);
    }

    if (Data->ValidBits.RequesterId != 0) {
        wprintf(L"      Requester ID:     0x%016I64x\n", Data->RequesterId);
    }

    if (Data->ValidBits.ResponderId != 0) {
        wprintf(L"      Responder ID:     0x%016I64x\n", Data->ResponderId);
    }

    if (Data->ValidBits.InstructionPointer != 0) {
        wprintf(L"      Instruction Ptr:  0x%016I64x\n", Data->InstructionPointer);
    }
}

VOID
DrDumpMemoryErrorSection (
    __in PWHEA_MEMORY_ERROR_SECTION Data
    )

/*++

Routine Description:

    This routine will dump the contents of the WHEA memory error section.

Arguments:

    Data - Supplies a pointer to the memory error section.

Return Value:

    None.

--*/

{

    const WCHAR *ErrorTypes[] = {
        L"Unknown",
        L"No Error",
        L"Single-bit ECC",
        L"Multi-bit ECC",
        L"Single-symbol Chipkill",
        L"Multi-symbol Chipkill",
        L"Master Abort",
        L"Target Abort",
        L"Parity Error",
        L"Watchdog Timeout",
        L"Invalid Address",
        L"Mirror Broken",
        L"Memory Sparing"
    };

    if (Data->ValidBits.ErrorStatus != 0) {
        wprintf(L"      Error Status:     0x%016I64x\n",
                Data->ErrorStatus.ErrorStatus);
    }

    if (Data->ValidBits.PhysicalAddress != 0) {
        wprintf(L"      Physical Address: 0x%016I64x\n",
                Data->PhysicalAddress);
    }

    if (Data->ValidBits.PhysicalAddressMask != 0) {
        wprintf(L"      Address mask:     0x%016I64x\n",
                Data->PhysicalAddressMask);
    }

    if (Data->ValidBits.Node != 0) {
        wprintf(L"      Node:             0x%x\n", Data->Node);
    }

    if (Data->ValidBits.Card != 0) {
        wprintf(L"      Card:             0x%x\n", Data->Card);
    }

    if (Data->ValidBits.Module != 0) {
        wprintf(L"      Module:           0x%x\n", Data->Module);
    }

    if (Data->ValidBits.Bank != 0) {
        wprintf(L"      Bank:             0x%x\n", Data->Bank);
    }

    if (Data->ValidBits.Device != 0) {
        wprintf(L"      Device:           0x%x\n", Data->Device);
    }

    if (Data->ValidBits.Row != 0) {
        wprintf(L"      Row:              0x%x\n", Data->Row);
    }

    if (Data->ValidBits.Column != 0) {
        wprintf(L"      Column:           0x%x\n", Data->Column);
    }

    if (Data->ValidBits.BitPosition != 0) {
        wprintf(L"      Bit position:     0x%x\n", Data->BitPosition);
    }

    if (Data->ValidBits.RequesterId != 0) {
        wprintf(L"      Requester ID:     0x%016I64x\n", Data->RequesterId);
    }

    if (Data->ValidBits.ResponderId != 0) {
        wprintf(L"      Responder ID:     0x%016I64x\n", Data->ResponderId);
    }

    if (Data->ValidBits.TargetId != 0) {
        wprintf(L"      Target ID:        0x%016I64x\n", Data->TargetId);
    }

    if (Data->ValidBits.ErrorType != 0) {
        wprintf(L"      Error Type:       %s\n",
                Data->ErrorType < RTL_NUMBER_OF(ErrorTypes) ?
                ErrorTypes[Data->ErrorType] : L"Invalid");
    }

    return;
}

VOID
DrDumpXpfMcaSection (
    __in PWHEA_XPF_MCA_SECTION Data
    )

/*++

Routine Description:

    This routine will dump the specified XPF MCA section.

Arguments:

    Data - Supplies a pointer to the MCA section.

Return Value:

    None.

--*/

{

    const WCHAR *CpuVendors[] = {
        L"Other", L"Intel", L"AMD"
    };

    WCHAR Buffer[80];

    wprintf(L"      CPU Vendor:       %s\n",
            Data->CpuVendor < RTL_NUMBER_OF(CpuVendors) ?
            CpuVendors[Data->CpuVendor] : L"Invalid");

    wprintf(L"      Processor Number: 0x%x\n",
            Data->ProcessorNumber);

    Buffer[0] = UNICODE_NULL;
    Buffer[1] = UNICODE_NULL;
    if (Data->GlobalStatus.RestartIpValid != 0) {
        wcscat_s(Buffer, 80, L" RIPV");
    }

    if (Data->GlobalStatus.ErrorIpValid != 0) {
        wcscat_s(Buffer, 80, L" EIPV");
    }

    if (Data->GlobalStatus.MachineCheckInProgress != 0) {
        wcscat_s(Buffer, 80, L" MCIP");
    }

    wprintf(L"      MCG_STATUS:       0x%016I64x (%s)\n",
            Data->GlobalStatus.QuadPart,
            &Buffer[1]);

    wprintf(L"      Instruction Ptr:  0x%016I64x\n", Data->InstructionPointer);
    wprintf(L"      MCA Bank:         0x%x\n", Data->BankNumber);

    Buffer[0] = UNICODE_NULL;
    Buffer[1] = UNICODE_NULL;
    if (Data->Status.Valid != 0) {
        wcscat_s(Buffer, 80, L" VAL");
    }

    if (Data->Status.StatusOverFlow != 0) {
        wcscat_s(Buffer, 80, L" OVER");
    }

    if (Data->Status.UncorrectedError != 0) {
        wcscat_s(Buffer, 80, L" UC");
    }

    if (Data->Status.ErrorEnabled != 0) {
        wcscat_s(Buffer, 80, L" EN");
    }

    if (Data->Status.MiscValid != 0) {
        wcscat_s(Buffer, 80, L" MISCV");
    }

    if (Data->Status.AddressValid != 0) {
        wcscat_s(Buffer, 80, L" ADDRV");
    }

    if (Data->Status.ContextCorrupt != 0) {
        wcscat_s(Buffer, 80, L" PCC");
    }

    wprintf(L"      MCi_STATUS:       0x%016I64x (%s)\n",
            Data->Status.QuadPart,
            &Buffer[1]);

    wprintf(L"          Other info:   0x%07x\n", Data->Status.OtherInformation);
    wprintf(L"          Model error:  0x%04x\n", Data->Status.ModelErrorCode);

    DrBitmap(Data->Status.McaErrorCode >> 12, 4, &Buffer[0], 5);
    Buffer[4] = L' ';
    DrBitmap(Data->Status.McaErrorCode >> 8, 4, &Buffer[5], 5);
    Buffer[9] = L' ';
    DrBitmap(Data->Status.McaErrorCode >> 4, 4, &Buffer[10], 5);
    Buffer[14] = L' ';
    DrBitmap(Data->Status.McaErrorCode, 4, &Buffer[15], 5);
    wprintf(L"          MCA error:    %s (binary)\n", Buffer);

    if (Data->Status.AddressValid != 0) {
        wprintf(L"      Address:          0x%016I64x\n", Data->Address);
    }

    if (Data->Status.MiscValid != 0) {
        wprintf(L"      Misc:             0x%016I64x\n", Data->Misc);
    }
}

PWSTR
DrWheaTime (
    __in const WHEA_TIMESTAMP *Timestamp,
    __out_ecount(BufferLength) PWSTR Buffer,
    __in DWORD BufferLength
    )

/*++

Routine Description:

    This routine will return the specified timestamp as a string formatted
    according to the current user's locale settings.

Arguments:

    Timestamp - Supplies the timestamp.

    Buffer - Supplies the buffer in which the string will be returned.

    BufferLength - Supplies the length in characters of the buffer.

Return Value:

    A pointer to BufferLength.

--*/

{

    WCHAR DateString[11];
    DWORD Length;
    int Result;
    SYSTEMTIME SystemTime;
    WCHAR TimeString[12];

    RtlZeroMemory(&SystemTime, sizeof(SYSTEMTIME));
    SystemTime.wYear = (WORD)((Timestamp->Century * 100) + Timestamp->Year);
    SystemTime.wMonth = (WORD)Timestamp->Month;
    SystemTime.wDay = (WORD)Timestamp->Day;
    SystemTime.wHour = (WORD)Timestamp->Hours;
    SystemTime.wMinute = (WORD)Timestamp->Minutes;
    SystemTime.wSecond = (WORD)Timestamp->Seconds;
    *Buffer = UNICODE_NULL;

    //
    // Convert the date portion of the time structure into a string formatted
    // according to the current user locale.
    //

    Result = GetDateFormatEx(NULL,
                             DATE_SHORTDATE,
                             &SystemTime,
                             NULL,
                             DateString,
                             RTL_NUMBER_OF(DateString),
                             NULL);

    if (Result == 0) {
        return Buffer;
    }

    //
    // Convert the time portion of the time structure into a string formatted
    // according to the current user locale.
    //

    Result = GetTimeFormatEx(NULL,
                             TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER,
                             &SystemTime,
                             NULL,
                             TimeString,
                             RTL_NUMBER_OF(TimeString));

    if (Result == 0) {
        return Buffer;
    }

    //
    // Ensure that the timestamp string will fit into the buffer.
    //

    Length = (DWORD)wcslen(DateString) +
        RTL_NUMBER_OF(L" ") +
        (DWORD)wcslen(TimeString);

    if (Length > BufferLength) {
        return Buffer;
    }

    //
    // Copy the results into the return buffer.
    //

    wcscpy_s(Buffer, BufferLength, DateString);
    wcscat_s(Buffer, BufferLength, L" ");
    wcscat_s(Buffer, BufferLength, TimeString);
    return Buffer;
}

PWSTR
DrWheaGuid (
    __in const GUID *Guid,
    __out_ecount(BufferLength) PWSTR Buffer,
    __in DWORD BufferLength
    )

/*++

Routine Description:

    This routine will display the specified GUID to the console. If the GUID is
    one of the well-known WHEA identifiers, then it will be displayed as a
    friendly string.

Arguments:

    Guid - Supplies a pointer to the GUID to be displayed.

Return Value:

    None.

--*/

{

    DWORD Count;
    int Return;
    PCWSTR String;
    UINT StringId;
    BOOL WellKnown;

    StringId = 0;
    WellKnown = FALSE;
    for (Count = 0; Count < RTL_NUMBER_OF(GuidLookup); Count += 1) {
        if (IsEqualGUID(Guid, GuidLookup[Count].Guid)) {
            StringId = GuidLookup[Count].StringId;
            WellKnown = TRUE;
            break;
        }
    }

    if (WellKnown != FALSE) {
        Return = LoadString(GetModuleHandle(NULL), StringId, (PWSTR)&String, 0);
        if (Return == 0) {
            WellKnown = FALSE;

        } else {
            swprintf_s(Buffer, BufferLength, L"%s", String);
        }
    }

    if (WellKnown == FALSE) {
        swprintf_s(Buffer,
                 BufferLength,
                 L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                 Guid->Data1,
                 Guid->Data2,
                 Guid->Data3,
                 Guid->Data4[0],
                 Guid->Data4[1],
                 Guid->Data4[2],
                 Guid->Data4[3],
                 Guid->Data4[4],
                 Guid->Data4[5],
                 Guid->Data4[6],
                 Guid->Data4[7]);
    }

    return Buffer;
}

PWSTR
DrWheaRecordFlagsString (
    __in const WHEA_ERROR_RECORD_HEADER_FLAGS Flags,
    __out_ecount(BufferLength) PWSTR Buffer,
    __in DWORD BufferLength
    )

/*++

Routine Description:

    This routine will render a friendly string for the specified GUID.

Arguments:

    Guid - Supplies a pointer to the GUID to be rendered.

    Buffer - Supplies a bugger in which the GUID will be rendered.

    BufferLength - Supplies the length in characters of the buffer.

Return Value:

    A pointer to the buffer.

--*/

{

    int Return;
    PCWSTR String;
    BOOL Separator;

    wcscpy_s(Buffer, BufferLength, L"none");
    Separator = FALSE;
    if (Flags.Recovered == 1) {
        Return = LoadString(GetModuleHandle(NULL),
                            IDS_WHEA_RECORD_FLAGS_RECOVERED,
                            (PWSTR)&String,
                            0);

        if (Return == 0) {
            String = L"Recovered";

        }

        wcscpy_s(Buffer, BufferLength, String);
        Separator = TRUE;
    }

    if (Flags.PreviousError == 1) {
        Return = LoadString(GetModuleHandle(NULL),
                            IDS_WHEA_RECORD_FLAGS_PREVIOUSERROR,
                            (PWSTR)&String,
                            0);

        if (Return == 0) {
            String = L"Previous Error";

        }

        if (Separator == FALSE) {
            *Buffer = UNICODE_NULL;

        } else {
            wcscat_s(Buffer, BufferLength, L", ");
        }

        wcscat_s(Buffer, BufferLength, String);
        Separator = TRUE;
    }

    if (Flags.Simulated == 1) {
        Return = LoadString(GetModuleHandle(NULL),
                            IDS_WHEA_RECORD_FLAGS_SIMULATED,
                            (PWSTR)&String,
                            0);

        if (Return == 0) {
            String = L"Simulated";

        }

        if (Separator == FALSE) {
            *Buffer = UNICODE_NULL;

        } else {
            wcscat_s(Buffer, BufferLength, L", ");
        }

        wcscat_s(Buffer, BufferLength, String);
    }

    return Buffer;
}

VOID
DrRcPrint (
    __in UINT FormatId,
    ...
    )

/*++

Routine Description:

    This routine functions in the same fashion as wprintf, except that instead
    of the format string being provided, a resource identifier is provided.

Arguments:

    FormatId - The resource identifier for the format string.

Return Value:

    None.

--*/

{

    va_list vargs;
    PCWSTR FormatString;
    int Return;

    va_start(vargs, FormatId);
    Return = LoadString(GetModuleHandle(NULL),
                        FormatId,
                        (PWSTR)&FormatString,
                        0);

    if (Return == 0) {
        return;
    }

    vwprintf_s(FormatString, vargs);
    va_end(vargs);
    return;
}

PWSTR
DrBitmap (
    __in ULONGLONG Value,
    __in ULONG Bits,
    __out_ecount(BufferLength) PWSTR Buffer,
    __in DWORD BufferLength
    )

/*++

Routine Description:

    This routine will convert the specified value into a binary bitmask.

Arguments:

    Value - Supplies the value to be converted.

    Bits - Supplies the number of bits to be produced.

    Buffer - Supplies a buffer in which the bitmask is returned.

    BufferLength - Supplies the buffer length.

Return Value:

    A pointer to the buffer.

--*/

{

    ULONG Count;
    ULONGLONG Mask;

    if (BufferLength <= Bits) {
        RtlZeroMemory(Buffer, BufferLength * sizeof(WCHAR));
        return Buffer;
    }

    Mask = 1UI64 << (Bits - 1);
    for (Count = 0; Count < Bits; Count += 1) {
        if ((Mask & Value) != 0) {
            Buffer[Count] = L'1';

        } else {
            Buffer[Count] = L'0';
        }

        Mask >>= 1;
    }

    Buffer[Count] = L'\0';
    return &Buffer[0];
}
