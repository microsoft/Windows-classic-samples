/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    common.h

Abstract:

    Definitions of formatting routines for various TDH in-types and out-types.

--*/

#ifdef __cplusplus
extern "C" {
#endif

#include <winsock2.h>
#include <windows.h>
#include <strsafe.h>
#include <intsafe.h>
#include <winnls.h>
#include <initguid.h>
#include <wmistr.h>
#include <evntcons.h>
#include <evntrace.h>
#include <sddl.h>
#include <In6addr.h>
#include <Ws2ipdef.h>

#ifdef __cplusplus
}
#endif

#include <comutil.h>

CONST ULONG STRLEN_GUID = 39;
CONST ULONG STRLEN_UTC_DATETIME = 64;
CONST ULONG ONE_HUNDRED_NANOSECONDS_PER_SECOND = 10000000;
CONST PWSTR FORMAT_DATE_STRING = L"yyyy'-'MM'-'dd";
CONST PWSTR FORMAT_TIME_STRING = L"HH':'mm':'ss";
CONST ULONG STRLEN_MAX_IPV6 = 50;
CONST ULONG STRLEN_MAX_SOCKADDR = 256;
CONST ULONG WIN7_MAJOR_VERSION = 6;
CONST ULONG WIN7_MINOR_VERSION = 1;


FORCEINLINE
VOID
Move64(
    __in PLARGE_INTEGER Src,
    __out PLARGE_INTEGER Dest
    )
{
    Dest->LowPart = Src->LowPart;
    Dest->HighPart = Src->HighPart;
}

ULONG
NullToBuffer(
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
AnsiStringToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT DataLentgh,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
UnicodeStringToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT DataLength,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );


ULONG
BooleanToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT DataLength,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
FormatDateTime(
    __in PSYSTEMTIME SystemTime,
    __out PBYTE Buffer
    );

ULONG
FileTimeToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
SystemTimeToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
GuidToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
SidToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
HexBinaryToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT PropertyLength,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
HexDumpToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
IPV6ToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT PropertyLength,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

BOOLEAN
IsOSPriorWin7(
    VOID
    );


template <typename T>
ULONG
NumberToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in PCWSTR FormatString,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This template method decodes an integer value and prints it 
    out to a memory buffer.

Arguments :

    BinDataPtr - Pointer to the undecoded payload buffer containing 
                 the integer value.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    DataLentgh - Length of the undecoded buffer in bytes.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value :

    ERROR_INSUFFICIENT_BUFFER - Buffer is not large enough. If this is returned,
                                the routine should be called again with a larger 
                                buffer.

    ERROR_SUCCESS - Formatting was successful.

--*/

{
    HRESULT hr;
    ULONG Status = ERROR_SUCCESS;
    PWCHAR StrEndPtr;

    T Value;
    USHORT DataSize = sizeof(T);
    
    if (DataSize > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    RtlCopyMemory(&Value, BinDataPtr, DataSize);

    //
    // Attempt to print Value to Buffer.
    //

    hr = StringCbPrintfExW((PWSTR)Buffer,
                           BufferSize,
                           &StrEndPtr,
                           NULL,
                           0,
                           FormatString,
                           Value);
    
    Status = HRESULT_CODE(hr);
    if (Status == ERROR_INVALID_PARAMETER) {
        Status = ERROR_INSUFFICIENT_BUFFER;
    }

    *BinDataConsumed = DataSize;
    return Status;
}
