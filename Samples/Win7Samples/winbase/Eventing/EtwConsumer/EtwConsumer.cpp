/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    EtwConsumer.cpp

Abstract:

    Sample consumer program for manifest-based events. EtwConsumer allows the user 
    to specify an ETL file containing events to be decoded and dumped to standard output. 
    This program supports two different dumping modes: XML or event message strings only.
    EtwConsumer will determine the Windows version of the decoding machine and will 
    use the appropriate TDH APIs accordingly. 

--*/

#include "TdhUtil.h"


ULONG
VPrintFToFile(
    __in BOOLEAN ForcePrint,
    __in PPROCESSING_CONTEXT LogContext,
    __in PWSTR FormatString,
    ...
    )

/*++

Routine Description:

    This routine prints to standard output the variable number of arguments passed in.
    All the printing from the sample is rerouted here.

Arguments:

    ForcePrint - Supplies the flag that indicates whether the content should be printed
                 independently from the state of LogContext.

    LogContext - Supplies the structure that holds the output memory buffer and
                 the flag that indicates whether printing should be performed.

    FormatString - Supplies the format in which the arguments will be printed.

    ... - Variable list of arguments to be printed.

Return Value:

    ERROR_SUCCESS - Arguments were successfully printed.

    ERROR_INSUFFICIENT_BUFFER - The buffer was too small for storing the output.

--*/

{

    INT StrLen;
    PWSTR Buffer;
    va_list Arguments;
    ULONG BufferSize;
    ULONG Status = ERROR_SUCCESS;

    if ((ForcePrint == FALSE) && (LogContext->DumpXml == FALSE)) {
        return ERROR_SUCCESS;
    }

    va_start(Arguments, FormatString);
    StrLen = _vscwprintf(FormatString, Arguments) + 1;
    
    if (StrLen >= 0 ) {
        BufferSize = StrLen * sizeof(WCHAR);
        if (LogContext->PrintBufferSize < BufferSize) {
            Status = ResizeBuffer(&LogContext->PrintBuffer, &LogContext->PrintBufferSize, BufferSize);
            if (Status != ERROR_SUCCESS) {
                return Status;
            }
        }
        Buffer = (PWSTR)LogContext->PrintBuffer;
        vswprintf_s(Buffer, StrLen, FormatString, Arguments);
        wprintf(L"%ls", Buffer);
    }

    va_end(Arguments);
    return ERROR_SUCCESS;
}

VOID
DumpEventHeader(
    __in PEVENT_RECORD Event,
    __in PTRACE_EVENT_INFO EventInfo,
    __inout PPROCESSING_CONTEXT LogContext
)

/*++

Routine Description:

    This routine dumps the metadata and the excecution parameters for the event.

Arguments:

    Event - Supplies the event whose header will be dumped.

    EventInfo - Supplies the information about the event whose header will be dumped.

    LogContext - Supplies processing context for the event.

Return Value:

    None.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    PEVENT_HEADER Header = (PEVENT_HEADER)&Event->EventHeader;
    LPGUID RelatedActivityID = NULL;
    PEVENT_EXTENDED_ITEM_TS_ID SessionID = NULL;
    PBYTE Sid = NULL;
    PEVENT_EXTENDED_ITEM_INSTANCE Instance = NULL;
    PULONG Stack32 = NULL;
    PULONG64 Stack64 = NULL;
    ULONG FrameCount = 0;
    LPGUID Guid;
    WCHAR GuidString[STRLEN_GUID];
    WCHAR DateTimeString[STRLEN_UTC_DATETIME];
    FILETIME  FileTime;
    ULONG GuidStringSize = STRLEN_GUID * sizeof(WCHAR);
    ULONG DateTimeStringSize = STRLEN_UTC_DATETIME * sizeof(WCHAR);
    USHORT Consumed;
    ULONG DataLeft = ULONG_MAX;


    for (ULONG i = 0; i < Event->ExtendedDataCount; i++) {

        switch (Event->ExtendedData[i].ExtType) {

        case EVENT_HEADER_EXT_TYPE_RELATED_ACTIVITYID:
            RelatedActivityID = (LPGUID)(Event->ExtendedData[i].DataPtr);
            break;

        case EVENT_HEADER_EXT_TYPE_SID:
            Sid = (PBYTE)(Event->ExtendedData[i].DataPtr);
            break;

        case EVENT_HEADER_EXT_TYPE_TS_ID:
            SessionID = (PEVENT_EXTENDED_ITEM_TS_ID)(Event->ExtendedData[i].DataPtr);
            break;

        case EVENT_HEADER_EXT_TYPE_INSTANCE_INFO:
            Instance = (PEVENT_EXTENDED_ITEM_INSTANCE)(Event->ExtendedData[i].DataPtr);
            break;

        case EVENT_HEADER_EXT_TYPE_STACK_TRACE32:
            Stack32 = (PULONG)(Event->ExtendedData[i].DataPtr);
            FrameCount = Event->ExtendedData[i].DataSize / 4;
            break;

        case EVENT_HEADER_EXT_TYPE_STACK_TRACE64:
            Stack64 = (PULONG64)(Event->ExtendedData[i].DataPtr);
            FrameCount = Event->ExtendedData[i].DataSize / 8;
            break;

        default:
            break;

        }
    }

    VPrintFToFile(FALSE, LogContext, L"\r\n\t<System>");

    //
    //  Provider element: Name is not printed for Wbem events since it
    //  can be localized and, hence, belongs in the <RenderingInfo> element.
    //

    VPrintFToFile(FALSE, LogContext, L"\r\n\t\t<Provider");

    if ((EventInfo != NULL) &&
        (IS_WBEM_EVENT(EventInfo) == 0) && 
        (TEI_PROVIDER_NAME(EventInfo) != NULL)) {

        VPrintFToFile(FALSE, LogContext, L" Name=\"%ls\"", TEI_PROVIDER_NAME(EventInfo));
    }
        
    Guid = &Event->EventHeader.ProviderId;
    if (Event->EventHeader.Flags & EVENT_HEADER_FLAG_CLASSIC_HEADER) {
        Guid = EventInfo ? &EventInfo->ProviderGuid : NULL;
    }

    if (Guid != NULL) {
        Status = GuidToBuffer((PBYTE)Guid, DataLeft, (PBYTE)&GuidString, GuidStringSize, &Consumed);
        if (Status == ERROR_SUCCESS) {
            VPrintFToFile(FALSE, LogContext, L" Guid=\"%ls", GuidString);
        }
    }
    VPrintFToFile(FALSE, LogContext, L"\" />");

    VPrintFToFile(FALSE,
                  LogContext,
                  L"\r\n\t\t<EventID>%u</EventID>"
                  L"\r\n\t\t<Version>%u</Version>"
                  L"\r\n\t\t<Level>%u</Level>"
                  L"\r\n\t\t<Task>%u</Task>"
                  L"\r\n\t\t<Opcode>%u</Opcode>"
                  L"\r\n\t\t<Keywords>0x%I64X</Keywords>",
                  Header->EventDescriptor.Id,
                  Header->EventDescriptor.Version,
                  Header->EventDescriptor.Level,
                  Header->EventDescriptor.Task,
                  Header->EventDescriptor.Opcode,
                  Header->EventDescriptor.Keyword);


    Move64(&Header->TimeStamp, (PLARGE_INTEGER)&FileTime);

    Status = FileTimeToBuffer((PBYTE)&FileTime, sizeof(FILETIME), (PBYTE)&DateTimeString[0], DateTimeStringSize, &Consumed);

    if (Status == ERROR_SUCCESS) {
        VPrintFToFile(FALSE, LogContext, L"\r\n\t\t<TimeCreated SystemTime=\"%ls\" />", DateTimeString);
    }

    //
    // ActivityId is in the event header.
    // RelatedActivityID is the ExtendedData of the event.
    //

    VPrintFToFile(FALSE, LogContext, L"\r\n\t\t<Correlation ");
    
    Status = GuidToBuffer((PBYTE)&Header->ActivityId, sizeof(GUID), (PBYTE)&GuidString, GuidStringSize, &Consumed);
    if (Status == ERROR_SUCCESS) {
        VPrintFToFile(FALSE, LogContext, L"ActivityID=\"%ls\"", (PWSTR)GuidString);
    }

    if (RelatedActivityID != NULL) {
        Status = GuidToBuffer((PBYTE)RelatedActivityID, sizeof(GUID), (PBYTE)&GuidString, ULONG_MAX, &Consumed);
        if (Status == ERROR_SUCCESS) {
                VPrintFToFile(FALSE, LogContext, L" RelatedActivityID=\"");
                VPrintFToFile(FALSE, LogContext, L"%ls\"", GuidString);
        }
    }

    VPrintFToFile(FALSE, LogContext, L" />");

    //
    // Execution parameters
    //

    VPrintFToFile(FALSE, 
                  LogContext, 
                  L"\r\n\t\t<Execution"
                  L" ProcessID=\"%u\""
                  L" ThreadID=\"%u\""
                  L" ProcessorID=\"%u\" ",
                  Header->ProcessId,
                  Header->ThreadId,
                  Event->BufferContext.ProcessorNumber);

    if (SessionID != NULL) {
        VPrintFToFile(FALSE, LogContext, L" SessionID=\"%lu\"", SessionID->SessionId);
    }

    if (LogContext->IsPrivateLogger != FALSE) {
        VPrintFToFile(FALSE, LogContext, L" KernelTime=\"%I64u\" />", Header->ProcessorTime);
    } else {
        VPrintFToFile(FALSE,
                      LogContext,
                      L" KernelTime=\"%lu\" UserTime=\"%lu\" />",
                      Header->KernelTime * LogContext->TimerResolution,
                      Header->UserTime * LogContext->TimerResolution);
    }


    //
    // For simplicity, the eventual call stack is not dumped. The call stack 
    // structure can be either EVENT_EXTENDED_ITEM_STACK_TRACE32 or
    // EVENT_EXTENDED_ITEM_STACK_TRACE64. These structures are pointed to by Stack32
    // and Stack64 variables, respectively, depending on machine architecture. 
    //

    VPrintFToFile(FALSE, LogContext, L"\r\n\t</System>");
}

ULONG
GetFormattedEventMessage(
    __in PTRACE_EVENT_INFO EventInfo,
    __in PWSTR* RenderItems,
    __out PWSTR* FormattedMessage
    )

/*++
    
Routine Description:

    This routine formats the original event message with the string 
    values obtained in the dumping process.

Arguments:

    EventInfo - Supplies the structure containing the original event message.

    RenderItems - Supplies an array of strings for the formatted toplevel properties.

    FormattedMessage - Receives the formatted string.

Return Value:

    ERROR_SUCCES - The formatting was successful or there was no data to format.

    Win32 error code - FormatMessageW() failed.

--*/

{
    ULONG Status = ERROR_SUCCESS;

    //
    // Get the original message (not the formatted event message), which may have 
    // references to the payload values of some of the top-level properties.
    // 

    PWSTR EventMessage = TEI_EVENT_MESSAGE(EventInfo);
    
    if (EventMessage != NULL && RenderItems != NULL) {
        ULONG Count = 0;
        Count = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_FROM_STRING |
                               FORMAT_MESSAGE_ARGUMENT_ARRAY,
                               (LPCVOID)EventMessage,
                               (ULONG)-1,
                               0,
                               (LPWSTR)FormattedMessage,
                               0,
                               (va_list*)RenderItems);

        if (Count == 0) {
            Status = GetLastError();
        }
    }
    return Status;
}

VOID
SaveReferenceValues(
    __in PEVENT_PROPERTY_INFO Property,
    __in USHORT PropertyIndex,
    __in PBYTE Data,
    __inout PPROCESSING_DATA_CONTEXT DataContext
    )

/*++

Routine Description:

    This routine caches the value of a simple property, which is not an array,
    in ReferenceValues. This value can be further referenced as a property
    length or array count.

Arguments:

    Property - Supplies the single property whose value will be cached.

    PropertyIndex - Supplies the index of the property whose value will be cached.

    Data - Supplies the raw byte property value to be cached.

    DataContext - Container of the cache (ReferenceValues).

Return Value:

    None.

--*/

{

    //
    // Only integer values can be cached. Find the integer type of the 
    // Property value and based on the type, do the proper formatting
    // and cache the result in DataContext->ReferenceValues.
    //

    USHORT InType = Property->nonStructType.InType;
    
    //
    // If Data is from a simple integer property whose value is NULL, ignore it.
    //

    if (Data == NULL) {
        return;
    }

    if (InType == TDH_INTYPE_UINT8) {

        UINT8 Value;
        RtlCopyMemory(&Value, Data, sizeof(UINT8));
        DataContext->ReferenceValues[PropertyIndex] = Value;

    } else if (InType == TDH_INTYPE_UINT16) {

        UINT16 Value;
        RtlCopyMemory(&Value, Data, sizeof(UINT16));
        DataContext->ReferenceValues[PropertyIndex] = Value;

    } else if ((InType == TDH_INTYPE_UINT32) || (InType == TDH_INTYPE_HEXINT32)) {

        UINT32 Value;
        RtlCopyMemory(&Value, Data, sizeof(UINT32));
        DataContext->ReferenceValues[PropertyIndex] = Value;
    }
}

USHORT
GetArrayCount(
    __in PEVENT_PROPERTY_INFO Property,
    __in PULONG ReferenceValues
    )

/*++
    
Routine Description:

    This routine retrieves the number of elements in a single property (simple 
    or complex).

Arguments:

    Property - Supplies the property whose number of elements will be retrieved.

    ReferenceValues - Supplies previously stored simple property values, which can
                      potentionally be referenced as a property length or array count.

Return Value:

    1 - When the property is not an array or an array with 1 member.

    Otherwise - The number of elements in the array.

--*/

{
    if ((Property->Flags & PropertyParamCount) != 0) {
        return (USHORT)ReferenceValues[Property->countPropertyIndex];
    } else {
        return Property->count;
    }
}

USHORT
GetPropertyLength(
    __in PEVENT_PROPERTY_INFO Property,
    __in PULONG ReferenceValues
    )

/*++
    
Routine Description:

    This routine retrieves the buffer length of Property.

Arguments:

    Property - Supplies the property whose buffer length will be retrieved.

    ReferenceValues - Supplies previously stored simple property values, which can
                      potentionally be referenced as a property length or array count.

Return Value:

    The length of the property buffer. 

--*/

{
    if ((Property->Flags & PropertyParamLength) != 0) {

        //
        // The property is not a fixed size property. Search the cache
        // ReferenceValues for its length.
        //

        return (USHORT)ReferenceValues[Property->lengthPropertyIndex];

    } else {

        //
        // The property is a fixed size property. Its length is stored
        // in the property information structure.
        //

        return Property->length;
    }
}

ULONG
FormatProperty(
    __in PEVENT_RECORD Event,
    __in PTRACE_EVENT_INFO EventInfo,
    __in_opt PEVENT_MAP_INFO EventMapInfo,
    __in PEVENT_PROPERTY_INFO Property,
    __in USHORT PropertyLength,
    __in ULONG PropertyIndex,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++
    
Routine Description:

    This routine prepares the formatting of the raw byte data contained 
    in the property buffer. First, the offset from Event->UserData is calculated.
    Then the data in that offset is passed for concrete formatting in 
    GetFormattedBuffer() or FormatMapToString(). These methods return the 
    formatted  buffer and the amount of binary data consumed and use the amount 
    of data consumed to advance the current data offset.

Arguments:

    Event - Supplies the structure representing an event.

    EventInfo - Supplies the event meta-information.

    Property - Supplies the property information about the simple property 
               to be decoded.

    PropertyLength - Supplies the length of the simple property to be decoded.

    PropertyIndex - Supplies the index of the property to be decoded.

    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:

    ERROR_SUCCESS - Success

    Win32 error code - Formatting the property data failed.

--*/
 
{
    ULONG Status = ERROR_SUCCESS;
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    ULONG BufferSize = DataContext->BufferSize;
    USHORT PointerSize;
    ULONG DataLeft;
    FPTR_TDH_FORMATPROPERTY TdhFormatPropertyPtr = NULL;

    DataContext->BinDataLeft = Event->UserDataLength - DataContext->UserDataOffset;
    PBYTE Data = (PBYTE)Event->UserData + DataContext->UserDataOffset;

    //
    // If no more data, just fill the buffer with one non-printable UNICODE_NULL.
    //

    if (DataContext->BinDataLeft == 0) {
        Status = NullToBuffer(DataContext->Buffer, DataContext->BufferSize, &DataContext->BinDataConsumed);
        if (Status == ERROR_SUCCESS) {
            UpdateRenderItem(DataContext);
        }
        return Status;
    }

    //
    // Get the pointer size on the machine where the event was fired.
    // Will be needed later when decoding certain types of properies.
    //

    if ((Event->EventHeader.Flags & EVENT_HEADER_FLAG_64_BIT_HEADER) != 0) {
        PointerSize = sizeof(ULONGLONG);
    } else if ((Event->EventHeader.Flags & EVENT_HEADER_FLAG_32_BIT_HEADER) != 0) {
        PointerSize = sizeof(ULONG);
    } else {
        PointerSize = (USHORT)LogContext->PointerSize;
    }
    
    do {

        if (Status == ERROR_INSUFFICIENT_BUFFER) {

            Status = ResizeBuffer(&DataContext->Buffer,
                                  &DataContext->BufferSize,
                                  ((DataContext->BufferSize / MIN_PROP_BUFFERSIZE) + 1) * MIN_PROP_BUFFERSIZE);

            if (Status != ERROR_SUCCESS) {
                return Status;
            }
        }

        //
        // Check if the Windows 7 TDH API routine, TdhFormatProperty(), is avaliable.
        //

        if ((LogContext->TdhDllHandle != NULL)) {
            TdhFormatPropertyPtr = LogContext->FormatPropertyPtr;
        }

        if (TdhFormatPropertyPtr != NULL) {

            //
            // The decoding process is on Windows 7 or later. In Windows 7, the TDH API
            // is updated with several new functions. One of them is TdhFormatProperty, which
            // deals with all valid TDH InTypes and OutTypes, and formats them properly.
            // In order to get the sample to compile on both Vista and Windows 7, load the 
            // TdhFormatProperty() dynamically.
            //

            Status = (*TdhFormatPropertyPtr)(EventInfo,
                                             EventMapInfo,
                                             PointerSize,
                                             Property->nonStructType.InType,
                                             Property->nonStructType.OutType,
                                             PropertyLength,
                                             DataContext->BinDataLeft,
                                             Data,
                                             &BufferSize,
                                             (PWSTR)DataContext->Buffer,
                                             &DataContext->BinDataConsumed);

        } else {
            
            //
            // The operating system is prior to Windows 7. The formatting for each 
            // InType and OutType property must be handled manually.
            //

            if (EventMapInfo == NULL) {

                //
                // This property has no map associated with it.  Directly pass the buffer 
                // referenced by the current offset for formatting to GetFormattedBuffer().
                // According to the in- and out-types of the property, proper formatting will 
                // be performed.
                //
     
                Status = GetFormattedBuffer(Data,
                                            DataContext->BinDataLeft,
                                            PropertyLength,
                                            PointerSize,
                                            Property->nonStructType.InType,
                                            Property->nonStructType.OutType,
                                            DataContext->Buffer,
                                            DataContext->BufferSize,
                                            &DataContext->BinDataConsumed);
            } else {

                //
                // This property has map associated with it. The map key value is 
                // in the Data buffer pointed by the property. It is a number pointing to 
                // some resource. GetFormattedMapValue() will find and format both the key
                // and its resource value and will return the formatted value as result.
                //

                Status = GetFormattedMapValue(Data,
                                              DataContext->BinDataLeft,  
                                              EventMapInfo,
                                              Property->nonStructType.InType,
                                              DataContext->Buffer,
                                              DataContext->BufferSize,
                                              &DataContext->BinDataConsumed);
            }
        }

    } while (Status == ERROR_INSUFFICIENT_BUFFER);

    if (Status == ERROR_EVT_INVALID_EVENT_DATA) {

        //
        // There can be cases when the string represented by the buffer Data, is 
        // not aligned with the event payload (i.e. it is longer than the actual data left).
        // Just copy and format the last DataContext->BinDataLeft bytes from the payload.
        //

        if (Property->nonStructType.InType == TDH_INTYPE_UNICODESTRING) {
            DataLeft = DataContext->BinDataLeft;
            if (DataContext->BufferSize < DataLeft) {
                Status = ResizeBuffer(&DataContext->Buffer,
                                      &DataContext->BufferSize,
                                      ((DataContext->BufferSize / MIN_PROP_BUFFERSIZE) + 1) * MIN_PROP_BUFFERSIZE);

                if (Status != ERROR_SUCCESS) {
                    return Status;
                }
            }
            RtlCopyMemory(DataContext->Buffer, Data, DataLeft);
            DataContext->Buffer[DataLeft] = 0;
            DataContext->Buffer[DataLeft + 1] = 0;
            DataContext->BinDataConsumed = (USHORT)DataLeft; 
            Status = ERROR_SUCCESS;

        } else if (Property->nonStructType.InType == TDH_INTYPE_ANSISTRING) {
            DataLeft = DataContext->BinDataLeft;
            BufferSize = (DataLeft + 1) * sizeof(WCHAR);
            if (DataContext->BufferSize < BufferSize) {

                Status = ResizeBuffer(&DataContext->Buffer,
                                      &DataContext->BufferSize,
                                      ((DataContext->BufferSize / MIN_PROP_BUFFERSIZE) + 1) * MIN_PROP_BUFFERSIZE);

                if (Status != ERROR_SUCCESS) {
                    return Status;
                }
            }

            DataContext->BinDataConsumed = (USHORT)MultiByteToWideChar(CP_ACP,
                                                                       0,
                                                                       (PSTR)Data,
                                                                       DataLeft,
                                                                       (PWSTR)DataContext->Buffer,
                                                                       DataLeft);
            
            DataLeft *= sizeof(WCHAR);
            DataContext->Buffer[DataLeft] = 0;
            DataContext->Buffer[DataLeft + 1] = 0;
            Status = ERROR_SUCCESS;

        } else if (EventMapInfo != NULL) {

            //
            // The integer key stored in Data was not matched as a valid map key entry.
            // Just try to print the formatted integer stored in Data.
            //

            Status = FormatProperty(Event,
                                    EventInfo,
                                    NULL,
                                    Property,
                                    PropertyLength,
                                    PropertyIndex,
                                    LogContext);

        }
    }

    if (Status == ERROR_SUCCESS) {
        DataContext->UserDataOffset += DataContext->BinDataConsumed;
        UpdateRenderItem(DataContext);
    }

    return Status;    
}

ULONG
CheckForMap(
    __in PEVENT_RECORD Event,
    __in PTRACE_EVENT_INFO EventInfo,
    __in PEVENT_PROPERTY_INFO Property,
    __inout PPROCESSING_CONTEXT LogContext,
    __out PEVENT_MAP_INFO* EventMapInfo
    )

/*++
    
Routine Description:

    This routine checks if there is any map associated with the 
    specified property from the passed event.

Arguments:

    Event - Supplies the structure representing an event.

    EventInfo - Supplies the event meta-information.

    Property - Supplies information about the simple property to be checked.

    EventMapInfo - Receives the resulting map information associated with the property.

Return Value:

    ERROR_SUCCESS - Success

    Win32 error code - TdhGetMapInformation() failed.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    PWSTR MapName = TEI_MAP_NAME(EventInfo, Property);
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    ULONG MapSize = DataContext->MapInfoBufferSize;

    if (MapName != NULL) {

        //
        // This property has a map associated with it. Try to
        // extract the information about that map, using TDH.
        //

        do {
            if (Status == ERROR_INSUFFICIENT_BUFFER) {
                Status = ResizeBuffer(&DataContext->MapInfoBuffer,
                                      &DataContext->MapInfoBufferSize,
                                      MapSize);

                if (DataContext->MapInfoBuffer == NULL) {
                    return ERROR_OUTOFMEMORY;
                }

                DataContext->MapInfoBufferSize = MapSize;
            }
            Status = TdhGetEventMapInformation(Event,
                                               MapName,
                                               (PEVENT_MAP_INFO)DataContext->MapInfoBuffer,
                                               &MapSize);

        } while (Status == ERROR_INSUFFICIENT_BUFFER);

        if (Status == ERROR_SUCCESS) {
            *EventMapInfo = (PEVENT_MAP_INFO)DataContext->MapInfoBuffer;
        }

    } else {
        *EventMapInfo = NULL;
    }
    return Status;
}

ULONG
DumpSimpleType(
    __in PEVENT_RECORD Event,
    __in PTRACE_EVENT_INFO EventInfo,
    __in PEVENT_PROPERTY_INFO Property,
    __in USHORT PropertyIndex,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++
    
Routine Description:

    This routine iterates over each property member in the
    simple property and passes it to the property formatting 
    function FormatProperty().  In case of single simple types, 
    only one iteration is performed.

Arguments:

    Event - Supplies the structure representing an event.

    EventInfo - Supplies the event meta-information.

    Property - Supplies the property information about the simple property 
               to be decoded.

    PropertyIndex - Supplies the index of the property to be decoded.

    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:

    ERROR_SUCCESS - Success.

    Win32 error code - Failure in obtaining the resulting map association for the property
                       or in formatting the property data.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    PEVENT_MAP_INFO EventMapInfo = NULL;
    USHORT ArrayCount;
    USHORT PropertyLength;
    USHORT InType = Property->nonStructType.InType;
    USHORT OutType = Property->nonStructType.OutType;
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    PBYTE Data = (PBYTE)Event->UserData + DataContext->UserDataOffset;
    
    //
    // Get the number of property elements. In the case where the property
    // is an array, the number of array members is stored in ArrayCount;
    // otherwise ArrayCount = 1.
    //

    ArrayCount = GetArrayCount(Property, DataContext->ReferenceValues);

    //
    // There are two special cases where the ArrayCount is equivalent to
    // the PropertyLength.
    //

    if (((InType == TDH_INTYPE_UNICODECHAR) || (InType == TDH_INTYPE_ANSICHAR)) &&
        (OutType == TDH_OUTTYPE_STRING)) {

        PropertyLength = ArrayCount;
        ArrayCount = 1;

    } else {

        PropertyLength = GetPropertyLength(Property, DataContext->ReferenceValues);
    }

    Status = CheckForMap(Event, EventInfo, Property, LogContext, &EventMapInfo);
    if (Status != ERROR_SUCCESS) {
        return Status;
    }

    //
    // Iterate through each member of the array represented by the simple property.
    // In the case of a simple single property, just format its data (ArrayCount = 1).
    //

    for (USHORT Counter = 0; Counter < ArrayCount; Counter++) {

        Status = FormatProperty(Event,
                                EventInfo,
                                EventMapInfo,
                                Property,
                                PropertyLength,
                                PropertyIndex,
                                LogContext);

        if (Status != ERROR_SUCCESS) {
            return Status;
        }
    
        //
        // The formatted property value is stored in DataContext->Buffer.
        //

        if (ArrayCount > 1) {
            VPrintFToFile(FALSE, LogContext,
                          L"\r\n\t\t<Data Name=\"%ls[%d]\">%ls</Data>",
                          TEI_PROPERTY_NAME(EventInfo, Property),
                          Counter,
                          (PWSTR)DataContext->Buffer);
        } else {
            VPrintFToFile(FALSE, LogContext,
                          L"\r\n\t\t<Data Name=\"%ls\">%ls</Data>",
                          TEI_PROPERTY_NAME(EventInfo, Property),
                          (PWSTR)DataContext->Buffer);
        }
    }


    if (ArrayCount == 1) {

        //
        // This is single simple single type (not an array), with the value stored 
        // in the Data variable (computed in FormatProperty). As it may be
        // referenced later, as some property length or array count, it should be cached
        // for eventual further useage.
        //

        SaveReferenceValues(Property, PropertyIndex, Data, DataContext);
    }

    return Status;
}

ULONG
DumpComplexType(
    __in PEVENT_RECORD Event,
    __in PTRACE_EVENT_INFO EventInfo,
    __in PEVENT_PROPERTY_INFO ComplexProperty,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++

Routine Description:

    This routine iterates over each simple property member in the complex 
    property, calculates its index, and passes it to DumpSimpleType().
    Complex properties can contain only simple properties (including arrays 
    as simple types).

Arguments:

    Event - Supplies the structure representing an event.

    EventInfo - Supplies the event meta-information.

    ComplexProperty - Supplies the property information about the complex property 
                      to be decoded.

    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:

    ERROR_SUCCESS - Success.

    Win32 error code - DumpSimpleType() failed.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    PEVENT_PROPERTY_INFO SimpleProperty;
    ULONG SimplePropertyCount;
    USHORT ArrayCount;
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    
    VPrintFToFile(FALSE,
                  LogContext,
                  L"\r\n\t\t<ComplexData Name=\"%ls\">",
                  TEI_PROPERTY_NAME(EventInfo, ComplexProperty));

    //
    // Get the number of structures if the the complex property is an 
    // array of structures.
    // N.B. A Complex property can be an array of structures, but cannot 
    // contain members that are structures.
    //

    ArrayCount = GetArrayCount(ComplexProperty, DataContext->ReferenceValues);
    
    for (USHORT I = 0; I < ArrayCount; I++) {
        SimplePropertyCount = ComplexProperty->structType.NumOfStructMembers;
        SimpleProperty = &EventInfo->EventPropertyInfoArray[ComplexProperty->structType.StructStartIndex];

        for (USHORT J = 0; J < SimplePropertyCount; J++, SimpleProperty++) {
            
            //
            // Dump the J-th simple member from the I-th structure. 
            //

            Status = DumpSimpleType(Event,
                                    EventInfo,
                                    SimpleProperty,
                                    ComplexProperty->structType.StructStartIndex + J,
                                    LogContext);

            if (Status != ERROR_SUCCESS) {
                return Status;
            }
        }
    }

    VPrintFToFile(FALSE, LogContext, L"\r\n\t\t</ComplexData>");

    return Status;
}

ULONG
DumpEventData(
    __in PEVENT_RECORD Event,
    __in PTRACE_EVENT_INFO EventInfo,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++

Routine Description:

    This routine iterates through each of the top-level properties from the event,
    decides if it is a complex or simple property, and delegates its dumping to the
    appropriate functions.

Arguments:

    Event - Supplies the structure representing an event.

    EventInfo - Supplies the event meta-information.

    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:

    ERROR_SUCCESS - Success.

    Win32 error code - The initial memory allocations or the dumper functions failed.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    PEVENT_PROPERTY_INFO Property;
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    
    DataContext->LastTopLevelIndex = -1;
    DataContext->BinDataLeft = Event->UserDataLength;
    DataContext->UserDataOffset = 0;

    if (EventInfo->TopLevelPropertyCount == 0) {
        DataContext->CurrentTopLevelIndex = -1;
        return ERROR_SUCCESS;
    }

    //
    // Allocated array of ULONGs for storing the simple integer property types.
    // This array can potentially be used for referencing some further property
    // array count or buffer length.
    //

    DataContext->ReferenceValuesCount = EventInfo->PropertyCount;
    DataContext->ReferenceValues = (PULONG)malloc(DataContext->ReferenceValuesCount * sizeof(LONG));
    if (DataContext->ReferenceValues == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Allocated array of strings, which will store the formatted values for each top-level
    // property. This array will be used in the end for formatting the event message. Alongside
    // this array, mantain another array of BOOLEANs which will hold the flags if some render
    // item was filled.
    //

    DataContext->RenderItemsCount = EventInfo->TopLevelPropertyCount;
    DataContext->RenderItems = (PWSTR*)malloc(DataContext->RenderItemsCount * sizeof(PWSTR));
    if (DataContext->RenderItems == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Iterate through each of the top-level properties and dump it acording to its type.
    //
 
    for (USHORT Index = 0; Index < EventInfo->TopLevelPropertyCount; Index++) {
        DataContext->CurrentTopLevelIndex = Index;
        Property = &EventInfo->EventPropertyInfoArray[Index];

        if (PROPERTY_IS_STRUCTURE(Property)) {
            Status = DumpComplexType(Event,
                                     EventInfo,
                                     Property,
                                     LogContext);

            if (Status != ERROR_SUCCESS) {
                break;
            }

        } else {
            Status = DumpSimpleType(Event,
                                    EventInfo,
                                    Property,
                                    Index,
                                    LogContext);
            
            if (Status != ERROR_SUCCESS) {
                break;
            }
        }
    }
    if (Status != ERROR_SUCCESS) {
        VPrintFToFile(FALSE, LogContext, L"\r\nError in decoding event payload.\n\n");
    }

    return Status;
}

ULONG
GetTraceEventInfo(
    __in PEVENT_RECORD Event,
    __inout PPROCESSING_CONTEXT LogContext,
    __out PTRACE_EVENT_INFO* EventInfo
    )

/*++

Routine Description:

    This routine retrieves the TRACE_EVENT_INFO structure for the
    passed EVENT_RECORD Event. This structure contains the meta-
    information about the event.

Arguments:

    Event - Supplies the structure representing an event.

    EventInfo - Receives the event meta-information.

Return Value:

    ERROR_SUCCESS - Success.

    Win32 error code - TdhGetEventInformation() failed.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    PPROCESSING_DATA_CONTEXT DataContext= &LogContext->DataContext;
    ULONG BufferSize = DataContext->EventInfoBufferSize;
    
    do {
        if (Status == ERROR_INSUFFICIENT_BUFFER) {
            Status = ResizeBuffer(&DataContext->EventInfoBuffer,
                                  &DataContext->EventInfoBufferSize,
                                  BufferSize);
            if (DataContext->EventInfoBuffer == NULL) {
                return ERROR_OUTOFMEMORY;
            }
            DataContext->EventInfoBufferSize = BufferSize;
        }

        Status = TdhGetEventInformation(Event,
                                        0,
                                        NULL,
                                        (PTRACE_EVENT_INFO)DataContext->EventInfoBuffer,
                                        &BufferSize);

    } while (Status == ERROR_INSUFFICIENT_BUFFER);
    
    if (Status == ERROR_SUCCESS) {
        *EventInfo = (PTRACE_EVENT_INFO)DataContext->EventInfoBuffer;
    }

    return Status;
}


ULONG
DumpEvent(
    __in PEVENT_RECORD Event,
    __in PPROCESSING_CONTEXT LogContext
    )

/*++

Routine Description:

    This routine decodes a single Event and prints it to standard output.
    First, the event header is dumped, then the event data, and lastly, 
    the formatted event message.

Arguments:

    Event - Supplies the structure representing an event.

    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:

    ERROR_SUCCESS - Success.

    Win32 error codes - Failure in the dumping process.

--*/

{

    PTRACE_EVENT_INFO EventInfo = NULL;
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    PWSTR EventMessage = NULL;

    ULONG Status = GetTraceEventInfo(Event, LogContext, &EventInfo);
    if (Status != ERROR_SUCCESS) {
        VPrintFToFile(TRUE, LogContext, L"\r\nError in retrieving event information. Possible corrupted installation on provider\n");
        return Status;
    }

    VPrintFToFile(FALSE, LogContext, L"\r\n<Event xmlns=\"http://schemas.microsoft.com/win/2004/08/events/event\">");

    //
    // If -xml output option was specified, dump the event header.
    //

    if (LogContext->DumpXml != FALSE) {
       DumpEventHeader(Event, EventInfo, LogContext);
    }
    

    VPrintFToFile(FALSE, LogContext, L"\r\n\t<EventData>");

    Status = DumpEventData(Event, EventInfo, LogContext);

    if (Status == ERROR_SUCCESS) {
        VPrintFToFile(FALSE, LogContext, L"\r\n\t</EventData>");
        VPrintFToFile(FALSE, LogContext, L"\r\n</Event>");

        Status = GetFormattedEventMessage(EventInfo, DataContext->RenderItems, &EventMessage);
        
        //
        // If the overall dumping process was successful, dump the formatted event message,
        // in the end. The message is dumped whether or not the -xml output option is specified
        //

        if (Status == ERROR_SUCCESS) {
            VPrintFToFile(TRUE, LogContext, L"\r\nEventMessage: %ls\n", EventMessage);
            if (EventMessage != NULL) {
                LocalFree(EventMessage);
            }
        }
    }
    
    //
    // Release the resources used for decoding the event payload.
    //

    ResetDataContext(DataContext);

    return Status;

}

VOID
WINAPI
EventCallback(
    __in PEVENT_RECORD Event
    )

/*++

Routine Description:

    This routine is called by ProcessTrace() for every event in the ETL file.
    It receives an EVENT_RECORD parameter, which contains the events header 
    and the event payload.

Arguments:

    Event - Supplies the structure that represents an Event.

Return Value:

    None.

--*/

{

    PPROCESSING_CONTEXT LogContext = (PPROCESSING_CONTEXT)Event->UserContext;

    if ((Event->EventHeader.ProviderId == EventTraceGuid) &&
        (Event->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO)) {

        //
        // First event in every file is a header event, some information
        // from which is needed to correctly decode subsequent events.
        //
        // N.B. This event is not available if consuming events in real-time mode.
        //
        
        PTRACE_LOGFILE_HEADER LogHeader = (PTRACE_LOGFILE_HEADER)Event->UserData;

        if (LogHeader != NULL) {

            LogContext->TimerResolution = LogHeader->TimerResolution;
            LogContext->PointerSize =  LogHeader->PointerSize;
            LogContext->IsPrivateLogger = (BOOLEAN)(LogHeader->LogFileMode & 
                                                    EVENT_TRACE_PRIVATE_LOGGER_MODE);
        }
        return;
    }

    if ((Event->EventHeader.Flags & EVENT_HEADER_FLAG_TRACE_MESSAGE) != 0) {

        //
        // Ignore WPP events.
        //

        return;
    }

    DumpEvent(Event, LogContext);

    LogContext->EventCount += 1;
}


ULONG
WINAPI
BufferCallback(
    __in PEVENT_TRACE_LOGFILE LogFile 
    )

/*++

Routine Description:

    An ETL file is divided into a number of buffers that contain events.
    This routine is called by ProcessTrace() after all the events in a buffer
    are delivered.

Arguments:

    LogFile -  A pointer to the structure that contains information
               about the buffer.

Return Value:

    TRUE - Continue processing.

    FALSE - Returning false cancels processing and ProcessTrace() returns.
            This is the only way to cancel processing. This sample will always
            return true.

--*/

{
    PPROCESSING_CONTEXT LogContext = (PPROCESSING_CONTEXT)LogFile->Context;

    LogContext->BufferCount += 1;

    return TRUE;
}


ULONG
DecodeFile(
    __in PWSTR FileName,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++

Routine Description:

    The main initialization on processing the ETL file is done here.
    First, a handle to the specified trace (the ETL file in this case)
    is obtained, then an ETW Api call, ProcessTrace(), is made. ProcessTrace()
    will invoke the Buffer and Event callback functions after processing
    each buffer and event, respectively. In the end, CloseTrace() is called.

Arguments:

    FileName - Supplies the name of the ETL file to be decoded.

    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:

    ERROR_SUCCESS - Success.

    Win32 error code - Calls to OpenTrace, ProcessTrace or CloseTrace failed.

--*/

{
    ULONG Status;
    EVENT_TRACE_LOGFILE LogFile = {0};
    TRACEHANDLE Handle;

    LogFile.LogFileName = FileName;
    LogFile.ProcessTraceMode |= PROCESS_TRACE_MODE_EVENT_RECORD;
    LogFile.EventRecordCallback = EventCallback;
    LogFile.BufferCallback = BufferCallback;
    LogFile.Context = (PVOID)LogContext;

    Handle = OpenTrace(&LogFile);
    if (Handle == INVALID_PROCESSTRACE_HANDLE) {
        Status = GetLastError();
        wprintf(L"\nOpenTrace failed. Error code: %u.\n", Status);
        return Status;
    }

    Status = ProcessTrace(&Handle, 1, NULL, NULL);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nProcessTrace failed. Error code: %u.\n", Status);
    }
    
    Status = CloseTrace(Handle);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nCloseTrace failed. Error code: %u.\n", Status);
    }

    return Status;
}

LONG
wmain(
    __in LONG argc,
    __in_ecount(argc) PWSTR* argv
    )

/*++

Routine Description:

    Main entry point for the sample. This sample takes an ETL file containing events
    and dumps the events to the screen. This sample can also take an additional switch for dumping 
    in XML format.

Arguments:

    argc - Supplies the argument count. Expected to be equal to 2 or 3.

    argv - Supplies the list of arguments. argv[1] should be path to an etl file.

Return Value:

    0 - Success.

    Win32 error code - Overall dumping process failed.

--*/

{
    ULONG Status;
    PROCESSING_CONTEXT LogContext;

    Status = InitializeProcessingContext(&LogContext);

    if (Status != ERROR_SUCCESS) {
        wprintf(L"\nThere was an error in the initialization.");
        return Status;
    }

    if ((argc == 1) || (argc > 3)) {
        wprintf(L"Usage: %s <etl file> [-xml]", argv[0]);
        return 1;
    } else if (argc == 3) {
        if (wcscmp(argv[2], L"-xml") == 0) {
            LogContext.DumpXml = TRUE;
        } else {
            wprintf(L"Invalid option %s\n", argv[2]);
        }
    }
    
    Status = DecodeFile(argv[1], &LogContext);

    if (Status == ERROR_SUCCESS) {

        wprintf(L"\n\nSummary:");
        wprintf(L"\n---------");
        wprintf(L"\nBuffers Processed : %u.", LogContext.BufferCount);
        wprintf(L"\nEvents Processed  : %I64u.", LogContext.EventCount);

    }

    return Status;
}
