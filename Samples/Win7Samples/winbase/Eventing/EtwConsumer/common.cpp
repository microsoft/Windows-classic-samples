/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    common.cpp

Abstract:

    Implementations of formatting routines for various TDH In-types and Out-types.

--*/

#include "common.h"

ULONG
NullToBuffer(
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine writes a single UNICODE_NULL character in the buffer.

Arguments:

    Buffer - Receives the formatted string.
    
    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Printing out the buffer failed.

--*/

{
    if (BufferSize < sizeof(WCHAR)) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    *(PWSTR)(&Buffer[0]) = UNICODE_NULL;
    *BinDataConsumed = 0;

    return ERROR_SUCCESS;
}

ULONG
AnsiStringToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT DataLentgh,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine decodes an ANSI string and prints it out to a memory
    buffer. If the length is not specified, this routine will take into
    account the possibility that the string may or may not contain a 
    terminating NULL character.

Arguments:

    BinDataPtr - Pointer to the undecoded payload.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    DataLentgh - Length of the string in bytes, if this is a fixed
                 length string or the length is parameterized by a previous
                 property in the payload. If the string is NULL-terminated,
                 length must include the NULL character.

    Buffer - Receives the formatted string.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/
{
    HRESULT hr;
    size_t StringByteLength;
    LONG CharsWritten;

    if (DataLentgh == 0) {

        hr = StringCbLengthA((PSTR)BinDataPtr,
                             BinDataLeft,
                             &StringByteLength);
        if (FAILED(hr)) {

            //
            // The string was longer than the actual event payload left to consume.
            //

            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        StringByteLength += sizeof(CHAR);

        if (BufferSize < StringByteLength * sizeof(WCHAR)) {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        CharsWritten = MultiByteToWideChar(CP_ACP,
                                           0,
                                           (PSTR)BinDataPtr,
                                           (LONG)StringByteLength,
                                           (PWSTR)Buffer,
                                           (LONG)StringByteLength);

        if (CharsWritten == 0) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        

        //
        // If the Ansi string was already null-terminated, that last
        // character is already counted in the consumption data.
        //

        if (BinDataPtr[StringByteLength - 2] == 0) {
            *BinDataConsumed = (USHORT)StringByteLength - 1;
        } else {
            *BinDataConsumed = (USHORT)StringByteLength;
        }

    } else {

        if (DataLentgh > BinDataLeft) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        StringByteLength = (DataLentgh + 1) * sizeof(WCHAR);

        if (BufferSize < StringByteLength) {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        CharsWritten = MultiByteToWideChar(CP_ACP,
                                           0,
                                           (PSTR)BinDataPtr,
                                           (LONG)DataLentgh,
                                           (PWSTR)Buffer,
                                           (LONG)DataLentgh);

        if (CharsWritten == 0) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        //
        // Manualy set the last character to NULL.
        //

        *((PWSTR)(Buffer + StringByteLength - sizeof(WCHAR))) = UNICODE_NULL;
        *BinDataConsumed = (USHORT)DataLentgh;
    }

    return ERROR_SUCCESS;
}

ULONG
UnicodeStringToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT DataLength,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine copies a Unicode string to a memory buffer. If length is 
    not specified, this routine accounts for the possibility that the string may or 
    may not include the terminating NULL character.

Arguments:

    BinDataPtr - Pointer to the undecoded payload.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    DataLentgh - Length of the string in bytes, if this is a fixed 
                 length string or the length is parameterized by a previous
                 property in the payload. If the string is NULL-terminated,
                 length must include the NULL character.

    Buffer - Receives the formatted string.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/

{
    ULONG StringByteLength;
    ULONG StringLength;
    PWSTR StartString;
    PWSTR EndString;
    PWSTR CurrentString;

    if (DataLength == 0) {

        StartString = (PWSTR)BinDataPtr;
        EndString = StartString + (BinDataLeft / 2);
        CurrentString = StartString;

        //
        // Get the length of the string, which may be misaligned
        // with the end of the event payload. If the end is reached without 
        // NULL terminating char found, the event data is invalid.
        //

        while ((CurrentString < EndString) && (*CurrentString != UNICODE_NULL)) {
            CurrentString++;
        }

        if (CurrentString == EndString) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }
        
        StringLength = (ULONG)(CurrentString - StartString);
        StringByteLength = (StringLength + 1) * sizeof(WCHAR);

        if (BufferSize < StringByteLength) {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        RtlCopyMemory(Buffer, BinDataPtr, StringByteLength);
        *BinDataConsumed = (USHORT)(StringByteLength);

    } else {
        StringByteLength = (DataLength + 1) * sizeof(WCHAR);

        if (DataLength > BinDataLeft) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        if (BufferSize < StringByteLength) {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        RtlCopyMemory(Buffer, BinDataPtr, StringByteLength);
        *((PWSTR)(Buffer + StringByteLength - sizeof(WCHAR))) = UNICODE_NULL;

        *BinDataConsumed = DataLength * sizeof(WCHAR);
    }

    return ERROR_SUCCESS;
}


ULONG
BooleanToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT DataLength,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine decodes a boolean value and prints it out 
    to a memory buffer.

Arguments:

    BinDataPtr - Pointer to the undecoded payload buffer containing the boolean value.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    DataLentgh - Length of the undecoded buffer in bytes.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/

{
    ULONG Value;
    PWSTR StringValue;
    ULONG StringByteLength;

    if (sizeof(ULONG) > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    RtlCopyMemory(&Value, BinDataPtr, DataLength);

    //
    // Define the string representation to be false or true.
    //

    StringValue = (Value == 0) ? L"false" : L"true";
    StringByteLength = ((ULONG)wcslen(StringValue) + 1) * sizeof(WCHAR);

    if (BufferSize < StringByteLength) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    RtlCopyMemory(Buffer, StringValue, StringByteLength);
    *((PWSTR)(Buffer + StringByteLength - sizeof(WCHAR))) = UNICODE_NULL;
    
    //
    // BOOLEAN is same as ULONG.
    //

    *BinDataConsumed = sizeof(ULONG);

    return ERROR_SUCCESS;

}

ULONG
FormatDateTime(
    __in PSYSTEMTIME SystemTime,
    __out PBYTE Buffer
    )

/*++

Routine Description:

    This routine prints out to a memory buffer the date and time 
    portions of a SYSTEMTIME structure.

Arguments:

    SystemTime - Supplies the time in SYSTEMTIME format.

    Buffer - The destination buffer where the printing is done.

Return Value:

    0 - Formatting was not successful.

    Otherwise - The size in bytes of printed characters.

--*/

{
    LONG StrLen;
    ULONG TotalLength = 0;

    //
    // First format the date part of SystemTime.
    //

    StrLen = GetDateFormatW(LOCALE_USER_DEFAULT,
                            0,
                            SystemTime,
                            FORMAT_DATE_STRING,
                            (PWSTR)Buffer,
                            STRLEN_UTC_DATETIME);

    if (StrLen == 0) {
        return 0;
    }
    
    TotalLength += StrLen;    

    //
    // Append the the time part of SystemTime.
    //
    
    *((PWSTR)(Buffer + (StrLen - 1) * sizeof(WCHAR))) = L'T';
    StrLen = GetTimeFormatW(LOCALE_USER_DEFAULT,
                            0,
                            SystemTime,
                            FORMAT_TIME_STRING,
                            (PWSTR)(Buffer + StrLen * sizeof(WCHAR)),
                            STRLEN_UTC_DATETIME);

    if (StrLen == 0) {
        return 0;
    }
    TotalLength += StrLen;

    return TotalLength * sizeof(WCHAR);
}

ULONG
FileTimeToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine decodes a FILETIME value and prints it 
    out to a memory buffer.

Arguments:

    BinDataPtr - Pointer to the undecoded payload buffer containing the FILETIME value.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/

{
    HRESULT Result;
    FILETIME FileTime;
    SYSTEMTIME SystemTime;
    ULARGE_INTEGER Time;
    ULONGLONG NanoSeconds;
    ULONG PrintedBytes;
    USHORT DataSize = sizeof(FILETIME);
    ULONG StringByteLength = STRLEN_UTC_DATETIME * sizeof(WCHAR);

    if (DataSize > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    if (BufferSize < StringByteLength) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    CopyMemory(&FileTime, BinDataPtr, sizeof(FILETIME));

    if ((FileTimeToSystemTime(&FileTime, &SystemTime) != FALSE) &&
        (SystemTime.wMonth <= 12)) {

        PrintedBytes = FormatDateTime(&SystemTime, Buffer);
        if (PrintedBytes == 0) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        //
        // Append NanoSeconds to the end of the formatted buffer.
        //

        CopyMemory(&Time, &FileTime, sizeof(FILETIME));
        NanoSeconds = (Time.QuadPart % ONE_HUNDRED_NANOSECONDS_PER_SECOND) * 100;
        Result = StringCchPrintfW((PWSTR)(Buffer + PrintedBytes - sizeof(WCHAR)),
                                  (STRLEN_UTC_DATETIME - PrintedBytes / sizeof(WCHAR)),
                                   L".%09I64uZ",
                                   NanoSeconds);

    } else {

        Result = StringCchPrintfW((PWSTR)Buffer,
                                  STRLEN_UTC_DATETIME,
                                  L"%u:%u",
                                  FileTime.dwLowDateTime,
                                  FileTime.dwHighDateTime);
    }

    if (FAILED(Result)) {
        return HRESULT_CODE(Result);
    }
    
    *BinDataConsumed = DataSize;

    return ERROR_SUCCESS;
}

ULONG
SystemTimeToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This template method decodes a SYSTEMTIME value and prints it 
    out to a memory buffer.

Arguments:

    BinDataPtr - Pointer to the undecoded payload buffer containing the SYSTEMTIME value.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/

{
    HRESULT Result;
    SYSTEMTIME SysTime;
    USHORT DataSize = sizeof(SYSTEMTIME);
    ULONG PrintedBytes;
    ULONG StringByteLength = STRLEN_UTC_DATETIME * sizeof(WCHAR);

    if (DataSize > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    if (BufferSize < StringByteLength) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    RtlCopyMemory(&SysTime, BinDataPtr, DataSize);

    if (SysTime.wMonth <= 12) {
        PrintedBytes = FormatDateTime(&SysTime, Buffer);

        if (PrintedBytes == 0) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        Result = StringCchPrintfW((PWSTR)(Buffer + PrintedBytes - sizeof(WCHAR)),
                                  (STRLEN_UTC_DATETIME - PrintedBytes / sizeof(WCHAR)),
                                  L".%03uZ",
                                  SysTime.wMilliseconds);


    } else {

        Result = StringCchPrintfW((PWSTR)Buffer,
                                  STRLEN_UTC_DATETIME,
                                  L"%u:%u:%u:%u:%u:%u:%u:%u",
                                  SysTime.wYear,
                                  SysTime.wMonth,
                                  SysTime.wDayOfWeek,
                                  SysTime.wDay,
                                  SysTime.wHour,
                                  SysTime.wMinute,
                                  SysTime.wSecond,
                                  SysTime.wMilliseconds);
    }

    if (FAILED(Result)) {
        return HRESULT_CODE(Result);
    }

    *BinDataConsumed = DataSize;

    return ERROR_SUCCESS;
}


ULONG
GuidToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine decodes a GUID value and prints it out to a memory buffer.

Arguments:

    BinDataPtr - Pointer to the undecoded payload buffer containing the GUID value.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/

{
    HRESULT Result;
    GUID Value;
    USHORT DataSize = sizeof(GUID);
    ULONG StringByteLength = STRLEN_GUID * sizeof(WCHAR); 

    if (DataSize > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    if (BufferSize < StringByteLength) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    RtlCopyMemory(&Value, BinDataPtr, DataSize);

    Result = StringCchPrintfW((PWSTR)Buffer,
                              STRLEN_GUID,
                              L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                              Value.Data1,
                              Value.Data2,
                              Value.Data3,
                              Value.Data4[0],
                              Value.Data4[1],
                              Value.Data4[2],
                              Value.Data4[3],
                              Value.Data4[4],
                              Value.Data4[5],
                              Value.Data4[6],
                              Value.Data4[7]);
    if (FAILED(Result)) {
        return HRESULT_CODE(Result);
    }
    
    *BinDataConsumed = DataSize;

    return ERROR_SUCCESS;
}

ULONG
SidToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine decodes a SID value and prints it out to a memory buffer.

Arguments:

    BinDataPtr - Pointer to the undecoded payload buffer containing the SID value.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/

{
    USHORT SidLength;
    PWSTR SidString;
    ULONG StringByteLength;
    
    if (sizeof(SID) > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    SidLength = 8 + (4 * BinDataPtr[1]);

    if (ConvertSidToStringSidW((PSID)BinDataPtr, &SidString)) {

        StringByteLength = ((ULONG)wcslen(SidString) + 1) * sizeof(WCHAR);

        if (BufferSize < StringByteLength) {
            LocalFree(SidString);
            return ERROR_INSUFFICIENT_BUFFER;
        }

        RtlCopyMemory(Buffer, (PBYTE)SidString, StringByteLength);
        LocalFree(SidString);

    } else {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    *BinDataConsumed = SidLength;

    return ERROR_SUCCESS;
}


ULONG
HexBinaryToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT PropertyLength,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine decodes a hex array, and prints it out to a memory buffer.

Arguments:

    BinDataPtr - Pointer to the undecoded payload buffer containing the hex binary.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    PropertyLength - The length of the binary in bytes.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/

{
    HRESULT Result;
    ULONG StringBytesLength;
    ULONG CurrentByteOffset = 0;

    if (PropertyLength == 0) {
        return NullToBuffer(Buffer, BufferSize, BinDataConsumed);
    }

    if (PropertyLength > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    //
    // Need to allocate 2 WCHARs for the hex prefix "0x", 
    // (PropertyLength * 2) WCHARs for each member of BinDataPtr
    // and one WCHAR for the NULL terminator.
    //

    StringBytesLength = 2 * sizeof(WCHAR) +
                        PropertyLength * 2 * sizeof(WCHAR)
                        + sizeof(WCHAR);

    if (BufferSize < StringBytesLength) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    Result = StringCbPrintfW((PWSTR)Buffer, StringBytesLength, L"0x");
    
    if (FAILED(Result)) {
        return HRESULT_CODE(Result);
    }

    CurrentByteOffset += 2 * sizeof(WCHAR); 

    //
    // Append each formatted hex member to Buffer.
    //

    for (USHORT Index = 0; Index < PropertyLength; Index++) {
        HRESULT hr = StringCbPrintfW((PWSTR)(Buffer + CurrentByteOffset),
                                     StringBytesLength - CurrentByteOffset,
                                     L"%02X",
                                     BinDataPtr[Index]);

        if (FAILED(hr)) {
            return HRESULT_CODE(hr);
        }

        //
        // Update the offset for receiving the next hex formatted value.
        //

        CurrentByteOffset +=  2 * sizeof(WCHAR);
    }

    *BinDataConsumed = PropertyLength;

    return ERROR_SUCCESS;
}

ULONG
HexDumpToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine decodes a hex array, and prints it out to a memory buffer.
    The difference with HexBinary data is that in the HexDump the binary
    length is specified in the first ULONG from the data.

Arguments:

    BinDataPtr - Pointer to the undecoded payload buffer containing the hex binary.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/
{
    ULONG Status = ERROR_SUCCESS;
    ULONG Length;
    USHORT PropertyLength;

    //
    //  Length is in the first ULONG.
    //

    if (sizeof(ULONG) > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    RtlCopyMemory(&Length, BinDataPtr, sizeof(ULONG));
    BinDataPtr += sizeof(ULONG);

    //
    //  N.B. Length is ULONG, however, it should not be bigger
    //      than USHORT_MAX.
    //

    if (Length > USHORT_MAX) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    PropertyLength = (USHORT)Length;

    if (PropertyLength == 0) {

        Status = NullToBuffer(Buffer, BufferSize, BinDataConsumed);
        *BinDataConsumed = sizeof(ULONG);

        return Status;
    }
    
    Status = HexBinaryToBuffer(BinDataPtr,
                               BinDataLeft - sizeof(ULONG),
                               PropertyLength,
                               Buffer,
                               BufferSize,
                               BinDataConsumed);

    if (Status != ERROR_SUCCESS) {
        return Status;
    }

    *BinDataConsumed += sizeof(ULONG);

    return Status;
}
ULONG
IPV6ToBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT PropertyLength,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++

Routine Description:

    This routine decodes an IP V6 address, and prints it out to a memory buffer.

Arguments:

    BinDataPtr - Pointer to the undecoded payload buffer containing 
                 the IP Version 6 address data.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    Buffer - Receives the formatted value.

    BufferSize - Supplies the size of Buffer in bytes.

    BinDataConsumed - Receives the data consumed from the event payload.

Return Value:

    ERROR_SUCCESS - Formatting was successful.

    Win32 error code - Formatting or printing out the buffer failed.

--*/

{

    //
    // Define a function pointer that will point to RtlIpv6AddressToStringW 
    // from ntdll.dll. Dynamically load ntdll.dll because the static library
    // is not included in the Microsoft SDK tools.
    //

    typedef LPTSTR (__stdcall *RtlIpv6AddressToStringProc)(IN6_ADDR*,LPTSTR);
    HINSTANCE NtDllHandle;
    RtlIpv6AddressToStringProc RtlIpv6AddressToStringW;
    
    IN6_ADDR Value;
    WCHAR IPV6Addr[STRLEN_MAX_IPV6];
    USHORT DataSize = sizeof(IN6_ADDR);

    if (PropertyLength == 0) {
        return NullToBuffer(Buffer, BufferSize, BinDataConsumed);
    }

    if (DataSize > BinDataLeft) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    //
    // Try to get a handle to ntdll.dll.
    //

    NtDllHandle = LoadLibraryA("ntdll.dll"); 
    if (NtDllHandle != NULL) { 

        //
        // Make handle to RtlIpv6AddressToStringW method from ntdll.dll.
        //

        RtlIpv6AddressToStringW = (RtlIpv6AddressToStringProc)GetProcAddress(NtDllHandle,
                                                                             "RtlIpv6AddressToStringW");
        if (RtlIpv6AddressToStringW != NULL)  {
            if (BufferSize < STRLEN_MAX_IPV6 * sizeof(WCHAR)) {
                return ERROR_INSUFFICIENT_BUFFER;
            }

            RtlCopyMemory(&Value, BinDataPtr, DataSize);

            //
            // Format the IPV6 value into a string.
            //

            (RtlIpv6AddressToStringW)(&Value, IPV6Addr);
            RtlCopyMemory(Buffer, (PBYTE)IPV6Addr, STRLEN_MAX_IPV6 * sizeof(WCHAR));

        } else {

            FreeLibrary(NtDllHandle);
            return ERROR_NOT_SUPPORTED;
        }

        FreeLibrary(NtDllHandle); 
    } else {
        return ERROR_NOT_SUPPORTED;
    }

    *BinDataConsumed = DataSize;

    return ERROR_SUCCESS;
}

BOOLEAN
IsOSPriorWin7(
    VOID
    )

/*++

Routine Description:

    This routine determines whether the version of the operating system is
    prior to Windows 7. This information is used to decide whether
    to use certain TDH API functions that are new in Windows 7.

Arguments:

    None.

Return Value:

    TRUE - The operating system is prior to Windows 7.

    FALSE - The operating system is Windows 7 or above.

--*/

{
    BOOL VersionQuerySuccess; 
    OSVERSIONINFO OperatingSystemInfo;

    ZeroMemory(&OperatingSystemInfo, sizeof(OSVERSIONINFO));
    OperatingSystemInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    VersionQuerySuccess = GetVersionEx(&OperatingSystemInfo);

    if (VersionQuerySuccess != FALSE) {
        if (OperatingSystemInfo.dwMajorVersion < WIN7_MAJOR_VERSION) {
            return TRUE;
        }
        if ((OperatingSystemInfo.dwMajorVersion == WIN7_MAJOR_VERSION) &&
            (OperatingSystemInfo.dwMinorVersion < WIN7_MINOR_VERSION)) {

            return TRUE;
        }
    }
    return FALSE;
}

