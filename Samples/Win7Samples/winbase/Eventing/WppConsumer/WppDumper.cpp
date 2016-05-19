/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    main.c

Abstract:

    Sample WPP events consumer program. Allows user to specify an etl
    file with WPP events and the corresponding tmf file, required
    for decoding the events.


--*/

#include "common.h"


BOOLEAN
FormatDateTime(
    __in PSYSTEMTIME SystemTime
    )

/*++

Routine Description:

    This routine prints out the date and time parts of a SYSTEMTIME
    structure.

Arguments:

    SystemTime - Supplies the time in SYSTEMTIME format.

Return Value:

    TRUE - Printing was successful.

    FALSE - Printing was not successful.

--*/

{
    WCHAR DateTime[STRLEN_UTC_DATETIME];
    LONG StrLen;

    StrLen = GetDateFormatW(LOCALE_USER_DEFAULT,
                           0,
                           SystemTime,
                           FORMAT_STRING_DATE,
                           DateTime,
                           STRLEN_UTC_DATETIME);

    if (StrLen == 0) {
        return FALSE;
    }

    wprintf(L"%s", DateTime);

    StrLen = GetTimeFormatW(LOCALE_USER_DEFAULT,
                           0,
                           SystemTime,
                           FORMAT_STRING_TIME,
                           DateTime,
                           STRLEN_UTC_DATETIME);

    if (StrLen == 0) {
        return FALSE;
    }

    wprintf(L"T%s", DateTime);

    return TRUE;
}


VOID
PrintWPPProperty(
    __in PEVENT_RECORD Event,
    __in USHORT InType,
    __in PWSTR PropertyName,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++

Routine Description:

    This routine prints a single property of a WPP event. WPP events have a
    fixed set of property names:

        SequenceNum.
        GuidName.
        FunctionName.
        FormattedString.
        ComponentName.
        SubComponentName.
        TraceGuid.
        GuidTypeName.
        SystemTime.
        FlagsName.
        LevelName.
        RawSystemTime.
        ProviderGuid.

Arguments:

    Event - Supplies the structure that represents an ETW event.

    InType - Type of the property to be printed.

    PropertyName - Name of the property to be printed.

    LogContext - Supplies the structure that persists contextual information
        across callbacks.

Return Value:

    None. On failure, the property does not get printed.

--*/

{
    PROPERTY_DATA_DESCRIPTOR PropertyData = {0};
    ULONG Status = ERROR_SUCCESS;

    PropertyData.ArrayIndex = ULONG_MAX;
    PropertyData.PropertyName = (ULONGLONG)PropertyName;

    do {

        if (Status == ERROR_INSUFFICIENT_BUFFER) {

            if (LogContext->Buffer != NULL) {
                free(LogContext->Buffer);
            }
            LogContext->Buffer = (PBYTE)malloc(LogContext->BufferSize * 2);
            if (LogContext->Buffer == NULL) {
                return;
            }
            LogContext->BufferSize *= 2;
        }

        Status = TdhGetProperty(Event,
                                _countof(LogContext->TdhContexts),
                                LogContext->TdhContexts,
                                1,
                                &PropertyData,
                                LogContext->BufferSize,
                                LogContext->Buffer);

    } while (Status == ERROR_INSUFFICIENT_BUFFER);

    if (Status != ERROR_SUCCESS) {
        return;
    }

    switch(InType) {

        case TDH_INTYPE_UNICODESTRING:
        {
            wprintf(L"%s", (PWSTR)LogContext->Buffer);
            break;
        }
        case TDH_INTYPE_UINT32:
        {
            ULONG Data;

            CopyMemory(&Data, LogContext->Buffer, sizeof(ULONG));
            wprintf(L"%u", Data);

            break;
        }
        case TDH_INTYPE_GUID:
        {
            GUID GuidValue;
            WCHAR GuidStr[STRLEN_GUID];

            CopyMemory(&GuidValue, LogContext->Buffer, sizeof(GUID));
            StringFromGUID2(GuidValue, GuidStr, STRLEN_GUID);
            wprintf(L"%s", GuidStr);

            break;
        }
        case TDH_INTYPE_FILETIME:
        {
            FILETIME FileTime;
            SYSTEMTIME SystemTime;
            ULARGE_INTEGER Time;
            ULONGLONG NanoSeconds;

            CopyMemory(&FileTime, LogContext->Buffer, sizeof(FILETIME));

            if (FileTimeToSystemTime(&FileTime, &SystemTime) &&
                (SystemTime.wMonth <= 12) &&
                FormatDateTime(&SystemTime)) {

                CopyMemory(&Time, &FileTime, sizeof(FILETIME));
                NanoSeconds = (Time.QuadPart % ONE_HUNDRED_NANOSECONDS_PER_SECOND) * 100;
                wprintf(L".%09I64uZ", NanoSeconds);

            } else {

                wprintf(L"%u:%u", FileTime.dwLowDateTime, FileTime.dwHighDateTime);
            }

            break;
        }
        case TDH_INTYPE_SYSTEMTIME:
        {
            SYSTEMTIME SystemTime;

            CopyMemory(&SystemTime, LogContext->Buffer, sizeof(SYSTEMTIME));

            if (SystemTime.wMonth <= 12) {

                if (FormatDateTime(&SystemTime)) {
                    wprintf(L".%03uZ", SystemTime.wMilliseconds);
                }

            } else {

                wprintf(L"%u:%u:%u:%u:%u:%u:%u:%u",
                        SystemTime.wYear,
                        SystemTime.wMonth,
                        SystemTime.wDayOfWeek,
                        SystemTime.wDay,
                        SystemTime.wHour,
                        SystemTime.wMinute,
                        SystemTime.wSecond,
                        SystemTime.wMilliseconds);
            }

            break;
        }
        default:
            break;
    }
}


VOID
ProcessWPPEvent(
    __in PEVENT_RECORD Event,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++

Routine Description:

    This routine decodes a single WPP event and prints it to console.

Arguments:

    Event - Supplies the structure that represents an ETW event.

    LogContext - Supplies the structure that persists contextual information
        across callbacks.

Return Value:

    None. On failure, certain properties don't get printed.

--*/

{
    wprintf(L"\n");

    wprintf(L"\nSequenceNumber: ");
    PrintWPPProperty(Event, TDH_INTYPE_UINT32, L"SequenceNum", LogContext);

    wprintf(L"\nGuidName: ");
    PrintWPPProperty(Event, TDH_INTYPE_UNICODESTRING, L"GuidName", LogContext);

    wprintf(L"\nFunction: ");
    PrintWPPProperty(Event, TDH_INTYPE_UNICODESTRING, L"FunctionName", LogContext);

    wprintf(L"\nMessage: ");
    PrintWPPProperty(Event, TDH_INTYPE_UNICODESTRING, L"FormattedString", LogContext);

    wprintf(L"\nComponent: ");
    PrintWPPProperty(Event, TDH_INTYPE_UNICODESTRING, L"ComponentName", LogContext);

    wprintf(L"\nSubComponent: ");
    PrintWPPProperty(Event, TDH_INTYPE_UNICODESTRING, L"SubComponentName", LogContext);

    wprintf(L"\nTraceGuid: ");
    PrintWPPProperty(Event, TDH_INTYPE_GUID, L"TraceGuid", LogContext);

    wprintf(L"\nFileLine: ");
    PrintWPPProperty(Event, TDH_INTYPE_UNICODESTRING, L"GuidTypeName", LogContext);

    wprintf(L"\nSystemTime: ");
    PrintWPPProperty(Event, TDH_INTYPE_SYSTEMTIME, L"SystemTime", LogContext);

    wprintf(L"\nFlags: ");
    PrintWPPProperty(Event, TDH_INTYPE_UNICODESTRING, L"FlagsName", LogContext);

    wprintf(L"\nLevel: ");
    PrintWPPProperty(Event, TDH_INTYPE_UNICODESTRING, L"LevelName", LogContext);

    wprintf(L"\nRawSystemTime: ");
    PrintWPPProperty(Event, TDH_INTYPE_FILETIME, L"RawSystemTime", LogContext);

    wprintf(L"\nProviderGuid: ");
    PrintWPPProperty(Event, TDH_INTYPE_GUID, L"ProviderGuid", LogContext);

}


VOID
ProcessHeaderEvent(
    __in PEVENT_RECORD Event,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++

Routine Description:

    This routine processes the header event, which is the first event in every
    log file, and contains some logfile-specific information. We will read the
    PointerSize property to find the pointer size of the platform on which the
    events were logged.

Arguments:

    Event - Supplies the structure that represents an ETW event.

    LogContext - Supplies the structure that persists contextual information
        across callbacks.

Return Value:

    None.

--*/

{
    ULONG Status = ERROR_SUCCESS;
    ULONG PointerSize;
    WCHAR PropertyName[] = L"PointerSize";
    PROPERTY_DATA_DESCRIPTOR PropertyData = {0};

    PropertyData.ArrayIndex = ULONG_MAX;
    PropertyData.PropertyName = (ULONGLONG)PropertyName;

    Status = TdhGetProperty(Event,
                            0,
                            NULL,
                            1,
                            &PropertyData,
                            sizeof(ULONG),
                            (PBYTE)&PointerSize);

    if (Status == ERROR_SUCCESS) {
        LogContext->TdhContexts[1].ParameterValue = (ULONGLONG)PointerSize;
    }
}


VOID
WINAPI
EventCallback(
    __in PEVENT_RECORD Event
    )

/*++

Routine Description:

    An ETL file is divided into a number of buffers, which contain events.
    This routine is called by ProcessTrace() for every event in an ETL file.

Arguments:

    Event - Supplies the structure that represents an ETW Event.

Return Value:

    None.

--*/

{
    PPROCESSING_CONTEXT LogContext = (PPROCESSING_CONTEXT)Event->UserContext;

    if ((Event->EventHeader.ProviderId == EventTraceGuid) &&
        (Event->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO)) {

        //
        // Header event is the special first event in every ETL file. We will
        // grab the pointersize of the platform on which the ETL was collected
        // from one of the properties of this event.
        //
        // N.B. This event is not available if consuming events in real-time mode.
        //

        ProcessHeaderEvent(Event, LogContext);
        return;
    }

    if ((Event->EventHeader.Flags & EVENT_HEADER_FLAG_TRACE_MESSAGE) == 0) {

        //
        // This sample only understands WPP events, ignore the rest.
        //

        return;
    }

    //
    // Prior to Win7, TdhGetEventInformation() needs to be called before 
    // TdhGetProperty(), although the return value of TdhGetEventInformation
    // is ignored. This call to TdhGetEventInformation() is not required on Win7.
    //
    
    if (LogContext->OSPriorWin7 != FALSE) {

        ULONG BufferSize = 0;
        TdhGetEventInformation(Event,
                               _countof(LogContext->TdhContexts),
                               LogContext->TdhContexts,
                               NULL,
                               &BufferSize);
    } 

    ProcessWPPEvent(Event, LogContext);

    LogContext->EventCount += 1;
}


ULONG
WINAPI
BufferCallback(
    __in PEVENT_TRACE_LOGFILE LogFile
    )

/*++

Routine Description:

    An ETL file is divided into a number of buffers, which contain events.
    This routine is called by ProcessTrace() after all the events in a buffer
    are delivered.

Arguments:

    LogFile - Supplies pointer to the structure that contains information
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
    __in LPWSTR FileName,
    __inout PPROCESSING_CONTEXT LogContext
    )

/*++

Routine Description:


Arguments:

    FileName - Supplies the name of the etl file to be decoded.

    LogContext - Supplies the structure that persists contextual information
        across callbacks.

Return Value:

    ERROR_SUCCESS on success.

    Win32 error code if calls to OpenTrace, ProcessTrace or CloseTrace fail.

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
    __in INT argc,
    __in_ecount(argc) LPWSTR* argv
    )

/*++

Routine Description:

    Main entry point for the sample. This sample takes an etl file with WPP events
    and a tmf file which has descriptions for all the WPP events in the etl file
    and dumps the events to screen.

Arguments:

    argc - Argument count. Expected to be equal to 3.

    argv - Arguments.
        argv[1] should be the name of an etl file.
        argv[2] should be the name of a tmf file.

Return Value:

    Status code, 0 on success.

--*/

{
    ULONG Status;
    PROCESSING_CONTEXT LogContext;
    
    if (argc != 3) {
        wprintf(L"Usage: %s <etl file> <tmf file>", argv[0]);
        return 1;
    }

    LogContext.TdhContexts[0].ParameterType = TDH_CONTEXT_WPP_TMFFILE;
    LogContext.TdhContexts[0].ParameterValue = (ULONGLONG)argv[2];

    //
    // Tdh needs to know the pointersize of the machine on which the etl
    // was collected. We will fill this up with the correct value later
    // by looking at the PointerSize property of the header event.
    //

    LogContext.TdhContexts[1].ParameterType = TDH_CONTEXT_POINTERSIZE;
    LogContext.TdhContexts[1].ParameterValue = (ULONGLONG)sizeof(PVOID);

    Status = DecodeFile(argv[1], &LogContext);

    if (Status == ERROR_SUCCESS) {

        wprintf(L"\n\nSummary:");
        wprintf(L"\n---------");
        wprintf(L"\nBuffers Processed : %u.", LogContext.BufferCount);
        wprintf(L"\nEvents Processed  : %I64u.", LogContext.EventCount);

    }

    return Status;
}
