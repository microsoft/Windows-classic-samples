/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    TdhUtil.cpp

Abstract:

   Implementations of the functions for rerouting the binary event data to proper 
   formatting routines based on operating system version. Also implements some other 
   utility functions like memory management functions, the printing function, and 
   dynamically loading tdh.dll.

--*/

#include "TdhUtil.h"


ULONG
GetFormattedBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT Length,
    __in USHORT PointerSize,
    __in USHORT InType,
    __in USHORT OutType,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++
    
Routine Description:

    This routine delegates the raw byte property buffer to the appropriate
    type-formatting function. It first determines the InType of the property. 
    Depending on the InType, the binary data is re-routed to different functions.
    In some properties, the OutType is also important for correct representation.

Arguments:

    BinDataPtr - Supplies the buffer to be formatted.

    BinDataLeft - Supplies the size of the event payload left to consume.

    Length - Supplies the length of the buffer to be formatted.

    PointerSize - Supplies the pointer size of the machine where the event was fired.

    InType - Supplies an integer value that represents the InType of the property.

    OutType - Supplies an integer value that represents the OutType of the property.

    Buffer - Receives the formatting string result.

    BufferSize - Supplies the size of the buffer to store the result.

    BinDataConsumed - Receives the number of bytes from BinDataPtr (used for formatting).

Return Value:

    ERROR_SUCCESS - Formatting was successful.
    
    Win32 error code - Formatting the property value failed.

--*/

{
    ULONG Status = ERROR_SUCCESS;

    //
    // With this switch statement, examine all valid TDH InTypes, and for the
    // one that matches the suplied parameter InType, re-route the binary data for 
    // further formatting.
    //

    switch (InType) {

    case TDH_INTYPE_UNICODESTRING:
    {
        switch(OutType) {

        case TDH_OUTTYPE_XML:
        case TDH_OUTTYPE_STRING:
        default:
            return UnicodeStringToBuffer(BinDataPtr, BinDataLeft, Length, Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_ANSISTRING:
    {
        switch(OutType) {

        case TDH_OUTTYPE_XML:
        case TDH_OUTTYPE_STRING:
        default:
            return AnsiStringToBuffer(BinDataPtr, BinDataLeft, Length, Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_INT8:
    {
        if (Length != sizeof(INT8)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }
        return NumberToBuffer<INT8>(BinDataPtr, BinDataLeft, L"%d", Buffer, BufferSize, BinDataConsumed);
    }
    case TDH_INTYPE_UINT8:
    {
        if (Length != sizeof(UINT8)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        switch(OutType) {

        case TDH_OUTTYPE_NOPRINT:
            Status = NullToBuffer(Buffer, BufferSize, BinDataConsumed);
            *BinDataConsumed = Length;
            return Status;

        case TDH_OUTTYPE_HEXINT8:
            return NumberToBuffer<UINT8>(BinDataPtr, BinDataLeft, L"0x%X", Buffer, BufferSize, BinDataConsumed);

        default:
            return NumberToBuffer<UINT8>(BinDataPtr, BinDataLeft, L"%u", Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_INT16:
    {
        if (Length != sizeof(INT16)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }
        return NumberToBuffer<INT16>(BinDataPtr, BinDataLeft, L"%hd", Buffer, BufferSize, BinDataConsumed);
    }
    case TDH_INTYPE_UINT16:
    {
        if (Length != sizeof(UINT16)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        switch(OutType) {

        case TDH_OUTTYPE_HEXINT16:
            return NumberToBuffer<UINT16>(BinDataPtr, BinDataLeft, L"0x%hX",  Buffer, BufferSize, BinDataConsumed);

        case TDH_OUTTYPE_PORT:

            //
            // Note: The integer value can be formatted like port number as well. 
            // For simplicity, in the sample, just the error code is printed.
            //

        default:
            return NumberToBuffer<UINT16>(BinDataPtr, BinDataLeft, L"%hu",  Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_INT32:
    {
        if (Length != sizeof(INT32)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        //
        // Note: The integer value can be formatted like error code as well. 
        // For simplicity, in the sample, just the error code value is printed.
        //

        return NumberToBuffer<INT32>(BinDataPtr, BinDataLeft, L"%I32d", Buffer, BufferSize, BinDataConsumed);
        
    }
    case TDH_INTYPE_UINT32:
    {
        if (Length != sizeof(UINT32)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        switch(OutType) {

        case TDH_OUTTYPE_HEXINT32:
        case TDH_OUTTYPE_ERRORCODE:
            return NumberToBuffer<INT32>(BinDataPtr, BinDataLeft, L"0x%I32X", Buffer, BufferSize, BinDataConsumed);

        case TDH_OUTTYPE_IPV4:
            
            //
            // Note: The integer value can be formatted like IPV4 address as well. 
            // For simplicity, in the sample, just the integer value is printed.
            //

        case TDH_OUTTYPE_WIN32ERROR:

            //
            // Note: The integer value can be formatted like Win32 error code as well. 
            // For simplicity, in the sample, just the integer value is printed.
            //

        case TDH_OUTTYPE_NTSTATUS:

            //
            // Note: The integer value can be formatted like NT status code as well. 
            // For simplicity, in the sample, just the integer value is printed.
            //

        case TDH_OUTTYPE_PID:
        case TDH_OUTTYPE_TID:

        default:
            return NumberToBuffer<INT32>(BinDataPtr, BinDataLeft, L"%I32u", Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_INT64:
    {
        if (Length != sizeof(INT64)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        return NumberToBuffer<INT64>(BinDataPtr, BinDataLeft, L"%I64d", Buffer, BufferSize, BinDataConsumed);
    }
    case TDH_INTYPE_UINT64:
    {
        if (Length != sizeof(UINT64)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        switch(OutType) {

        case TDH_OUTTYPE_HEXINT64:
            return NumberToBuffer<UINT64>(BinDataPtr, BinDataLeft, L"0x%I64X", Buffer, BufferSize, BinDataConsumed);

            //
            // Note: The integer value can be formatted like ETWTIME code as well.
            // For simplicity, in the sample, just the integer value is printed.
            //

        case TDH_OUTTYPE_ETWTIME:

        default:
            return NumberToBuffer<UINT64>(BinDataPtr, BinDataLeft, L"%I64u", Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_FLOAT:
    {
        if (Length != sizeof(FLOAT)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        return NumberToBuffer<FLOAT>(BinDataPtr, BinDataLeft, L"%f", Buffer, BufferSize, BinDataConsumed);
    }
    case TDH_INTYPE_DOUBLE:
    {
        if (Length != sizeof(DOUBLE)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        return NumberToBuffer<DOUBLE>(BinDataPtr, BinDataLeft, L"%f", Buffer, BufferSize, BinDataConsumed);
    }
    case TDH_INTYPE_BOOLEAN:
    {
        return BooleanToBuffer(BinDataPtr, BinDataLeft, Length, Buffer, BufferSize, BinDataConsumed);
    }
    case TDH_INTYPE_BINARY:
    {
        switch(OutType) {

        case TDH_OUTTYPE_IPV6:
            return IPV6ToBuffer(BinDataPtr, BinDataLeft, Length, Buffer, BufferSize, BinDataConsumed);

        case TDH_OUTTYPE_SOCKETADDRESS:

            //
            // Note: The binary data can be formatted like socket address as well. 
            // For simplicity, in the sample, just the raw binary data is printed.
            //

        case TDH_OUTTYPE_HEXBINARY:

        default:
            return HexBinaryToBuffer(BinDataPtr, BinDataLeft, Length, Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_GUID:
    {
        if (Length < sizeof(GUID)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        return GuidToBuffer(BinDataPtr, BinDataLeft, Buffer, BufferSize, BinDataConsumed);
    }
    case TDH_INTYPE_POINTER:
    case TDH_INTYPE_SIZET:
    {
        if (Length != PointerSize) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        if (PointerSize == 4) {
            return NumberToBuffer<UINT32>(BinDataPtr, BinDataLeft, L"0x%I32X", Buffer, BufferSize, BinDataConsumed);
        } else {
            return NumberToBuffer<UINT64>(BinDataPtr, BinDataLeft, L"0x%I64X", Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_FILETIME:
        if (Length < sizeof(FILETIME)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        return FileTimeToBuffer(BinDataPtr, BinDataLeft,  Buffer, BufferSize, BinDataConsumed);

    case TDH_INTYPE_SYSTEMTIME:
        if (Length < sizeof(SYSTEMTIME)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        return SystemTimeToBuffer(BinDataPtr, BinDataLeft, Buffer, BufferSize, BinDataConsumed);

    case TDH_INTYPE_SID:
        return SidToBuffer(BinDataPtr, BinDataLeft, Buffer, BufferSize, BinDataConsumed);

    case TDH_INTYPE_HEXINT32:
    {
        if (Length != sizeof(UINT32)) {
            return ERROR_EVT_INVALID_EVENT_DATA;
        }

        switch(OutType) {

        case TDH_OUTTYPE_WIN32ERROR:

            //
            // Note: The integer value can be formatted like Win32 error code as well. 
            // For simplicity, in the sample, just the integer value is printed.
            //

        case TDH_OUTTYPE_NTSTATUS:

            //
            // Note: The integer value can be formatted like NT status as well. 
            // For simplicity, in the sample, just the integer value is printed.
            //

        default:
            return NumberToBuffer<UINT32>(BinDataPtr, BinDataLeft, L"0x%I32X", Buffer, BufferSize, BinDataConsumed);
        }
    }

    case TDH_INTYPE_HEXINT64:
        return NumberToBuffer<UINT64>(BinDataPtr, BinDataLeft, L"0x%I64X", Buffer, BufferSize, BinDataConsumed);

    case TDH_INTYPE_COUNTEDSTRING:
    case TDH_INTYPE_COUNTEDANSISTRING:
    case TDH_INTYPE_REVERSEDCOUNTEDSTRING:
    case TDH_INTYPE_REVERSEDCOUNTEDANSISTRING:

        //
        // These are legacy MOF string types and are not implemented in this sample.
        //

        return ERROR_NOT_SUPPORTED;
        

    case TDH_INTYPE_NONNULLTERMINATEDSTRING:
        return UnicodeStringToBuffer(BinDataPtr, BinDataLeft, 0, Buffer, BufferSize, BinDataConsumed);

    case TDH_INTYPE_NONNULLTERMINATEDANSISTRING:
        return AnsiStringToBuffer(BinDataPtr, BinDataLeft, Length, Buffer, BufferSize, BinDataConsumed);

    case TDH_INTYPE_UNICODECHAR:
    {
        switch(OutType) {

        case TDH_OUTTYPE_STRING:

            //
            //  N.B.: Caller needs to make sure that the count for this property is
            //        treated as length.
            //
            return UnicodeStringToBuffer(BinDataPtr, BinDataLeft, Length, Buffer, BufferSize, BinDataConsumed);

        default:
            return NumberToBuffer<WCHAR>(BinDataPtr, BinDataLeft, L"%c", Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_ANSICHAR:
    {
        switch(OutType) {

        case TDH_OUTTYPE_STRING:

            //
            //  N.B.: Caller needs to make sure that the count for this property is
            //        treated as length.
            //
            
            return AnsiStringToBuffer(BinDataPtr, BinDataLeft, Length, Buffer, BufferSize, BinDataConsumed);

        default:
            return NumberToBuffer<CHAR>(BinDataPtr, BinDataLeft, L"%C", Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_HEXDUMP:
    {
        switch(OutType) {

        case TDH_OUTTYPE_HEXBINARY:
        default:
            return HexDumpToBuffer(BinDataPtr, BinDataLeft, Buffer, BufferSize, BinDataConsumed);
        }
    }
    case TDH_INTYPE_WBEMSID:
    {
        return ERROR_NOT_SUPPORTED;
    }
    case TDH_INTYPE_NULL:
    {
        
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    default:
        Status = ERROR_NOT_SUPPORTED;
        break;
    }

    return Status;
}



ULONG
BitMapToString(
    __in PEVENT_MAP_INFO MapInfo,
    __in ULONG Value,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++
    
Routine Description:

    This routine maps value to string. Each bit from the value binary representation
    can match a map entry.

Arguments:

    MapInfo - Supplies information about the map.

    Value - Supplies the bit key value from the map.

    Buffer - Receives the formatted map value string result.

    BufferSize - Supplies the size of the buffer that should store the result.

    BinDataConsumed - Receives the number of bytes from used BinDataPtr (used for formatting).

Return Value:

    ERROR_SUCCESS - The formatting was successful.
    
    Win32 error code - Formatting the map value failed.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    BOOLEAN Match;
    BOOLEAN AtLeastOneMatchFound = FALSE;
    PWCHAR MapString;
    PEVENT_MAP_ENTRY MapEntry;
    ULONG DataLeft = ULONG_MAX;

    //
    // Iterate through each map entry (the key values), to find out
    // which ones match key value provided. Thus, one key value can 
    // point to several map entries. In this case, a search for the 
    // first match is performed.
    //

    for (ULONG i = 0; i < MapInfo->EntryCount; i++) {

        MapEntry = &MapInfo->MapEntryArray[i];

        //
        // In EVENTMAP_INFO_FLAG_MANIFEST_BITMAP case, if Value matches at
        // least one bit with the MapEntry value, then it is matched.
        // In EVENTMAP_INFO_FLAG_WBEM_VALUEMAP, the MapEntry value is the 
        // position of the bit that would be eventually set in Value.
        //

        if (MapInfo->Flag == EVENTMAP_INFO_FLAG_MANIFEST_BITMAP) {
            Match = (BOOLEAN)(MapEntry->Value & Value);
        } else {
            Match = (BOOLEAN)((1 << MapEntry->Value) & Value);
        }

        if (Match != FALSE) {
            MapString = EMI_MAP_OUTPUT(MapInfo, MapEntry);
            if (MapString != NULL) {
                Status = UnicodeStringToBuffer((PBYTE)MapString,
                                               DataLeft,
                                               (USHORT)wcslen(MapString),
                                               Buffer,
                                               BufferSize,
                                               BinDataConsumed);
                AtLeastOneMatchFound = TRUE;
                break;
            }
        }
    }

    if (AtLeastOneMatchFound == FALSE) {
        Status = ERROR_EVT_INVALID_EVENT_DATA;
    }

    return Status;
}

ULONG
ValueMapToString(
    __in PEVENT_MAP_INFO MapInfo,
    __in ULONG Value,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++
    
Routine Description:

    This routine maps an integer value to string.

Arguments:

    MapInfo - Supplies information about the map.

    Value - Supplies the integer key value from the map.

    Buffer - Receives the formatted map value string result.

    BufferSize - Supplies the size of the buffer that should store the result.

    BinDataConsumed - Receives the number of bytes from used BinDataPtr (used for formatting).

Return Value:

    ERROR_SUCCESS - The formatting was successful.
    
    Win32 error code - Formatting the map value failed.

--*/

{
    ULONG Status = ERROR_EVT_INVALID_EVENT_DATA;
    PEVENT_MAP_ENTRY MapEntry;
    PWSTR MapString;
    ULONG DataLeft = ULONG_MAX;

    //
    // Iterate over each map entry (the key value), to find
    // the one that match the parameter Value, which is the
    // key that is searched.
    //

    for (ULONG i = 0; i < MapInfo->EntryCount; i++) {
        MapEntry = &MapInfo->MapEntryArray[i];
        if (MapEntry->Value == Value) {

            //
            // Get the string value that coresponds to the MapEntry.
            //

            MapString = EMI_MAP_OUTPUT(MapInfo, MapEntry);
            if (MapString != NULL) {
                
                //
                // Since MapString is not part of the event payload (but its resource), the information
                // about binary data left is not important in this call. Just pass ULONG_MAX value.
                //

                Status = UnicodeStringToBuffer((PBYTE)MapString,
                                               DataLeft,
                                               (USHORT)wcslen(MapString),
                                               Buffer,
                                               BufferSize,
                                               BinDataConsumed);
            }

            //
            // The key was found; stop searching.
            //

            break;
        }
    }

    return Status;
}

ULONG
MapToString(
    __in PEVENT_MAP_INFO MapInfo,
    __in ULONG Value,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++
    
Routine Description:

    This routine uses the flag of the map determine its type. There can be
    Value maps, Bit maps, and ManifestPattern maps.

Arguments:

    MapInfo - Supplies information about the map.

    Value - Supplies the integer key value of the map.

    Buffer - Receives the formatted map value string result.

    BufferSize - Supplies the size of the buffer that should store the result.

    BinDataConsumed - Receives the number of bytes from used BinDataPtr (used for formatting).

Return Value:

    ERROR_SUCCESS - The formatting was successful.
    
    Win32 error code - Formatting the map value failed.

--*/

{

    switch (MapInfo->Flag) {

    case EVENTMAP_INFO_FLAG_MANIFEST_VALUEMAP:
    case EVENTMAP_INFO_FLAG_WBEM_VALUEMAP:
        return ValueMapToString(MapInfo, Value, Buffer, BufferSize, BinDataConsumed);

    case EVENTMAP_INFO_FLAG_MANIFEST_BITMAP:
    case EVENTMAP_INFO_FLAG_WBEM_BITMAP:
        return BitMapToString(MapInfo, Value, Buffer, BufferSize, BinDataConsumed);

    case EVENTMAP_INFO_FLAG_MANIFEST_PATTERNMAP:
        return ERROR_NOT_SUPPORTED;

    default:
        return ERROR_NOT_SUPPORTED;
    }
}


template <typename T>
ULONG
FormatMap(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in PEVENT_MAP_INFO MapInfo,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++
    
Routine Description:

    This routine copies the key binary value into a ULONG variable. This 
    variable is passed to another routine, which gets the map value for 
    the given key value.

Arguments:

    BinDataPtr - Supplies the buffer that should be formatted.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    MapInfo - Supplies information about the map.

    Buffer - Receives the formatted map value string result.

    BufferSize - Supplies the size of the buffer that should store the result.

    BinDataConsumed - Receives the number of bytes from used BinDataPtr (used for formatting).

Return Value:

    ERROR_SUCCESS - The formatting was successful.
    
    Win32 error code - Formatting the map value failed.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    T Value;
    USHORT DataSize = sizeof(T);
    
    if (BinDataLeft < DataSize) {
        return ERROR_EVT_INVALID_EVENT_DATA;
    }

    RtlCopyMemory(&Value, BinDataPtr, DataSize);
    Status = MapToString(MapInfo, Value, Buffer, BufferSize, BinDataConsumed);

    return Status;
}

ULONG
GetFormattedMapValue(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in PEVENT_MAP_INFO MapInfo,
    __in USHORT InType,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    )

/*++
    
Routine Description:

    This routine delegates the raw byte property buffer (which is the key value of the map)
    to the appropriate map-formatting function. It first determines the InType of the property. 
    Depending on the InType, the binary data is re-routed to different fucntions.

Arguments:

    BinDataPtr - Supplies the buffer that should be formatted.

    BinDataLeft - Supplies the size of the event payload data left to consume.

    MapInfo - Supplies information about the map.

    InType - Supplies an integer value that represents the InType of the property,

    Buffer - Receives the formatted map value string result.

    BufferSize - Supplies the size of the buffer that should store the result.

    BinDataConsumed - Receives the number of bytes from used BinDataPtr (used for formatting).

Return Value:

    ERROR_SUCCESS - The formatting was successful.
    
    Win32 error code - Formatting the map value failed.

--*/


{
    ULONG Status = ERROR_SUCCESS;

    //
    // Determine what kind of integer is the key value of the map, and delegate
    // to the template map formatting function.
    //

    switch (InType) {

    case TDH_INTYPE_UINT8:
        Status = FormatMap<UINT8>(BinDataPtr, BinDataLeft, MapInfo, Buffer, BufferSize, BinDataConsumed);
        break;

    case TDH_INTYPE_UINT16:
        Status = FormatMap<UINT16>(BinDataPtr, BinDataLeft, MapInfo, Buffer, BufferSize, BinDataConsumed);
        break;

    case TDH_INTYPE_UINT32:
    case TDH_INTYPE_HEXINT32:
        Status = FormatMap<UINT32>(BinDataPtr, BinDataLeft, MapInfo, Buffer, BufferSize, BinDataConsumed);
        break;

    default:
        return ERROR_EVT_INVALID_EVENT_DATA;
        break;
    }

    return Status;
}


VOID
GetFormatPropertyHandle(
    __out HMODULE* TdhLibraryHandle,
    __out FPTR_TDH_FORMATPROPERTY* FormatPropertyPtr
    )
/*++
    
Routine Description:

    This routine dynamically loads TdhFormatProperty() from tdh.dll. It is called
    in the case when the operating system is Windows 7 or above.

Arguments:

    TdhLibraryHandle - Receives the module handle to tdh.dll.

    FormatPropertyPtr - Receives the function address of TdhFormatProperty from tdh.dll.

Return Value:

    None.
--*/

{
    *TdhLibraryHandle = LoadLibraryW(L"tdh.dll");

    if (*TdhLibraryHandle == NULL) {
        return;
    }

    *FormatPropertyPtr = (FPTR_TDH_FORMATPROPERTY)GetProcAddress(*TdhLibraryHandle,
                                                                 "TdhFormatProperty");
}

ULONG
InitializeProcessingContext(
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++
    
Routine Description:

    This routine initializes the memory buffers in one processing context, and the
    data context structure contained in it. Additionally, this routine attempts to 
    dynamically load TdhFormatProperty() from tdh.dll, if the operating system is 
    Windows 7 or above.

Arguments:

    LogContext - Supplies a structure with the buffers that should be allocated and 
                 the handle and function pointer to tdh.dll and TdhFormatProperty(), 
                 respectively.

Return Value:

    ERROR_SUCCESS - The memory allocations were successful.
    
    ERROR_OUTOFMEMORY - There was insufficient memory.

--*/

{

    ULONG Status = InitializeDataContext(&LogContext->DataContext);
    
    if (Status != ERROR_SUCCESS) {
        return Status;
    }

    LogContext->PrintBuffer = (PBYTE)malloc(LogContext->PrintBufferSize);
    if (LogContext->PrintBuffer == NULL) {
        Status = ERROR_OUTOFMEMORY;
    }

    if (IsOSPriorWin7() == FALSE) {
        GetFormatPropertyHandle(&LogContext->TdhDllHandle, &LogContext->FormatPropertyPtr);
    }
    
    return ERROR_SUCCESS;
}


ULONG
InitializeDataContext(
    __inout PPROCESSING_DATA_CONTEXT DataContext
    )

/*++
    
Routine Description:

    This routine initializes the memory buffers in one PROCESSING_DATA_CONTEXT structure.
    

Arguments:

    DataContext - Supplies the structure with the buffers that should be allocated. 

Return Value:

    ERROR_SUCCESS - The memory allocations were successful.
    
    ERROR_OUTOFMEMORY - There was insufficient memory.

--*/

{

    DataContext->Buffer = (PBYTE)malloc(DataContext->BufferSize);
    if (DataContext->Buffer == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    DataContext->MapInfoBuffer = (PBYTE)malloc(DataContext->MapInfoBufferSize);
    if (DataContext->MapInfoBuffer == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    DataContext->EventInfoBuffer = (PBYTE)malloc(DataContext->EventInfoBufferSize);
    if (DataContext->EventInfoBuffer == NULL) {
        return ERROR_OUTOFMEMORY;
    }
    
    return ERROR_SUCCESS;
}

VOID
ResetDataContext(
    __inout PPROCESSING_DATA_CONTEXT DataContext
    )

/*++
    
Routine Description:

    This routine releases the resources needed for decoding one event.
    It is called after each event is decoded.

Arguments:

    DataContext - Supplies the data context whose resources should be released.

Return Value:

    None.

--*/

{

    //
    // The last top-level index was CurrentTopLevelIndex. If it was -1, that
    // means that there were no propertires in the event, and nothing has 
    // been allocated.
    //

    if (DataContext->CurrentTopLevelIndex == -1) {
        return;
    }
    
    if (DataContext->RenderItems != NULL) {
        for(LONG Index = 0; Index < DataContext->CurrentTopLevelIndex; Index++) {
            if (DataContext->RenderItems[Index] != NULL) {
                free(DataContext->RenderItems[Index]);
            }
        }
        free(DataContext->RenderItems);
    }

    if (DataContext->ReferenceValues != NULL) {
        free(DataContext->ReferenceValues);
    }
}

ULONG
ResizeBuffer(
    __inout PBYTE* Buffer,
    __out PULONG BufferSize,
    __in ULONG NewBufferSize
    )

/*++
    
Routine Description:

    This routine resizes the supplied buffer.

Arguments:

    Buffer - Supplies buffer that should be resized.

    BufferSize - Supplies old size of the buffer that should be resized.
    
    NewBufferSize - Supplies the new size of the buffer.

Return Value:

    ERROR_SUCCESS - Resizing was successful.
    
    ERROR_OUTOFMEMORY - There was insufficient memory.

--*/

{

    if (*Buffer != NULL) {
        free(*Buffer);
    }

    *Buffer = (PBYTE)malloc(NewBufferSize);
    if (*Buffer == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    *BufferSize = NewBufferSize;

    return ERROR_SUCCESS;
}

ULONG
UpdateRenderItem(
    __inout PPROCESSING_DATA_CONTEXT DataContext
    )

/*++
    
Routine Description:

    This routine saves the formatted value of the property identified 
    by CurrentTopLevelIndex.

Arguments:

    DataContext - Supplies the data context of the formatted property 
                  that should be saved.

Return Value:

    None.

--*/

{
    HRESULT Result;
    ULONG StringLength;
    PLONG LastIndex = &DataContext->LastTopLevelIndex;
    LONG CurrentIndex = DataContext->CurrentTopLevelIndex;

    //
    // In the case of arrays and structures, there is one render string as well.
    // For simplicity, in the sample, do not accumulate all their members, but just
    // store the first one. 
    //

    if (*LastIndex != CurrentIndex) {

        StringLength = (ULONG)wcslen((PWSTR)DataContext->Buffer) + 1;
        DataContext->RenderItems[CurrentIndex] = (PWSTR)malloc(StringLength * sizeof(WCHAR));
        if (DataContext->RenderItems[CurrentIndex] == NULL) {
            return ERROR_OUTOFMEMORY;
        }

        Result = StringCchPrintfW(DataContext->RenderItems[CurrentIndex],
                                  StringLength,
                                  L"%s",
                                  (PWSTR)DataContext->Buffer);
        if (FAILED(Result)) {
            return HRESULT_CODE(Result);
        }

        *LastIndex = CurrentIndex;
    }

    return ERROR_SUCCESS;
}
