/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    tracelog.c

Abstract:

    Sample trace control program. Allows user to start, update, query, stop 
    event tracing, etc.


--*/ 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <guiddef.h>
#include <evntrace.h>

#define MAXSTR 1024

//
// Default trace file name.
//

#define DEFAULT_LOGFILE_NAME _T("C:\\LogFile.Etl")

//
// On Windows 2000, we support up to 32 loggers at once.
// On Windows XP and .NET server, we support up to 64 loggers. 
//

#define MAXIMUM_LOGGERS 64

//
// In this sample, we support the following actions. 
// Additional actions that we do not use in this sample include 
// Flush and Enumerate Guids functionalities. They are supported
// only on XP or higher version.
//

#define ACTION_QUERY 0
#define ACTION_START 1
#define ACTION_STOP 2
#define ACTION_UPDATE 3
#define ACTION_LIST 4
#define ACTION_ENABLE 5
#define ACTION_HELP 6

#define ACTION_UNDEFINED 10

VOID
PrintLoggerStatus(
    __in PEVENT_TRACE_PROPERTIES LoggerInfo,
    __in ULONG Status
    );

ULONG 
HexToLong(
    __in TCHAR *String
    );

VOID
StringToGuid(
    __in TCHAR *String,
    __out LPGUID Guid
    );

VOID 
PrintHelpMessage(
    VOID
    );

 
LONG
main(
    __in INT argc,
    __in_ecount(argc) LPTSTR* argv
    )

/*++

Routine Description:

    It is the main function.

Arguments:

    argc - Number of the arguments passed to the command line.

    argv - Array of strings which holds each argument value.
  
Return Value:

    Error Code defined in winerror.h : If the function succeeds, 
    it returns ERROR_SUCCESS(=0).

--*/

{
    ULONG LoggerCounter;
    ULONG Status = ERROR_SUCCESS;
    LPTSTR *targv;
    LPTSTR *utargv = NULL;

    //
    // Action to be taken
    //

    USHORT Action = ACTION_UNDEFINED;

    LPTSTR LoggerName;
    LPTSTR LogFileName;

    //
    // We will store the custom logger settings in LoggerInfo.
    //
       
    PEVENT_TRACE_PROPERTIES LoggerInfo;

    TRACEHANDLE LoggerHandle = 0;

    //
    // Target GUID, level and flags for enable/disable
    //

    GUID TargetGuid = {0};
    BOOLEAN TargetGuidProvided = FALSE;    

    ULONG Enable = TRUE;

    ULONG SizeNeeded = 0;

    //
    // We will enable Process, Thread, Disk, and Network events 
    // if the Kernel Logger is requested.
    //

    BOOL IsKernelLogger = FALSE;
    
    //
    // Allocate and initialize EVENT_TRACE_PROPERTIES structure first.
    //

    SizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) +
                 2 * MAXSTR * sizeof(TCHAR);

    LoggerInfo = (PEVENT_TRACE_PROPERTIES)malloc(SizeNeeded);
    if (LoggerInfo == NULL) {
        return (ERROR_OUTOFMEMORY);
    }
    
    RtlZeroMemory(LoggerInfo, SizeNeeded);

    LoggerInfo->Wnode.BufferSize = SizeNeeded;
    LoggerInfo->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 
    LoggerInfo->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    LoggerInfo->LogFileNameOffset = LoggerInfo->LoggerNameOffset +
                                    MAXSTR * sizeof(TCHAR);

    LoggerName = (LPTSTR)((PCHAR)LoggerInfo + LoggerInfo->LoggerNameOffset);
    LogFileName = (LPTSTR)((PCHAR)LoggerInfo + LoggerInfo->LogFileNameOffset);

    //
    // If the logger name is not given, we will assume the kernel logger.
    //
    
    _tcscpy_s(LoggerName, MAXSTR, KERNEL_LOGGER_NAME);

#ifdef UNICODE
    UNREFERENCED_PARAMETER(argv);
    if ((targv = CommandLineToArgvW(GetCommandLineW(),
                                    &argc)) == NULL) {
        free(LoggerInfo);
        return (GetLastError());
    };
    utargv = targv;
#else
    targv = argv;
#endif

    //
    // Parse the command line options to determine actions and parameters.
    //

    while (--argc > 0) {
        ++targv;
        if (**targv == '-' || **targv == '/') {  // argument found
            if (targv[0][0] == '/' ) {
                targv[0][0] = '-';
            }
            
            //
            // Determine actions.
            //

            if (_tcsicmp(targv[0], _T("-start")) == 0) {
                Action = ACTION_START;
                if (argc > 1) {
                    if ((targv[1][0] != '-') && (targv[1][0] != '/')) {
                        ++targv;
                        --argc;
                        _tcscpy_s(LoggerName, MAXSTR, targv[0]);
                    }
                }
            } else if (_tcsicmp(targv[0], _T("-enable")) == 0) {
                Action = ACTION_ENABLE;
                if (argc > 1) {
                    if ((targv[1][0] != '-') && (targv[1][0] != '/')) {
                        ++targv;
                        --argc;
                        _tcscpy_s(LoggerName, MAXSTR, targv[0]);
                    }
                }
            } else if (_tcsicmp(targv[0], _T("-disable")) == 0) {
                Action = ACTION_ENABLE;
                Enable = FALSE;
                if (argc > 1) {
                    if ((targv[1][0] != '-') && (targv[1][0] != '/')) {
                        ++targv;
                        --argc;
                        _tcscpy_s(LoggerName, MAXSTR, targv[0]);
                    }
                }
            } else if (_tcsicmp(targv[0], _T("-stop")) == 0) {
                Action = ACTION_STOP;
                if (argc > 1) {
                    if ((targv[1][0] != '-') && (targv[1][0] != '/')) {
                        ++targv;
                        --argc;
                        _tcscpy_s(LoggerName, MAXSTR, targv[0]);
                    }
                }
            } else if (_tcsicmp(targv[0], _T("-update")) == 0) {
                Action = ACTION_UPDATE;
                if (argc > 1) {
                    if ((targv[1][0] != '-') && (targv[1][0] != '/')) {
                        ++targv;
                        --argc;
                        _tcscpy_s(LoggerName, MAXSTR, targv[0]);
                    }
                }
            } else if (_tcsicmp(targv[0], _T("-query")) == 0) {
                Action = ACTION_QUERY;
                if (argc > 1) {
                    if ((targv[1][0] != '-') && (targv[1][0] != '/')) {
                        ++targv;
                        --argc;
                       _tcscpy_s(LoggerName, MAXSTR, targv[0]);
                    }
                }
            } else if (_tcsicmp(targv[0], _T("-list")) == 0) {
                Action  = ACTION_LIST;
            }
 
            //
            // Get other parameters.
            // Users can customize logger settings further by adding/changing 
            // values to LoggerInfo. Refer to EVENT_TRACE_PROPERTIES 
            // documentation for available options.
            // In this sample, we allow changing maximum number of buffers and
            // specifying user mode (private) logger.
            // We also take trace file name and guid for enable/disable.
            //

            else if (_tcsicmp(targv[0], _T("-f")) == 0) {
                if (argc > 1) {
                    _tfullpath(LogFileName, targv[1], MAXSTR);
                    ++targv;
                    --argc;
                }
            } else if (_tcsicmp(targv[0], _T("-guid")) == 0) {
                if (argc > 1) {

                    // 
                    // Before the guid value, we expect "#"
                    // -guid #xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
                    //

                    if (targv[1][0] == _T('#')) {
                        StringToGuid(&targv[1][1], &TargetGuid);
                        TargetGuidProvided = TRUE;
                        ++targv;
                        --argc;
                    }
                }
            } else if (_tcsicmp(targv[0], _T("-max")) == 0) {
                if (argc > 1) {
                    LoggerInfo->MaximumBuffers = _ttoi(targv[1]);
                    ++targv;
                    --argc;
                }
            } else if (_tcsicmp(targv[0], _T("-um")) == 0) {
                LoggerInfo->LogFileMode |= EVENT_TRACE_PRIVATE_LOGGER_MODE;

            } else if ((targv[0][1] == 'h') ||
                       (targv[0][1] == 'H') ||
                       (targv[0][1] == '?')) {

                Action = ACTION_HELP;
                PrintHelpMessage();

                if (utargv != NULL) {
                    GlobalFree(utargv);
                }

                free(LoggerInfo);

                return (ERROR_SUCCESS);

            } else {
                Action = ACTION_UNDEFINED;
            }
        } else { 
            _tprintf(_T("Invalid option given: %s\n"), targv[0]);

            Status = ERROR_INVALID_PARAMETER;
            goto CleanUpAndExit;
        }
    }

    //
    // Set the kernel logger parameters.
    //

    if (_tcscmp(LoggerName, KERNEL_LOGGER_NAME) == 0) {

        //
        // Set enable flags. Users can add options to add additional kernel events 
        // or remove some of these events.
        //

        LoggerInfo->EnableFlags |= EVENT_TRACE_FLAG_PROCESS;
        LoggerInfo->EnableFlags |= EVENT_TRACE_FLAG_THREAD;
        LoggerInfo->EnableFlags |= EVENT_TRACE_FLAG_DISK_IO;
        LoggerInfo->EnableFlags |= EVENT_TRACE_FLAG_NETWORK_TCPIP;

        LoggerInfo->Wnode.Guid = SystemTraceControlGuid; 
        IsKernelLogger = TRUE;

    } else if (LoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {

        //
        // We must provide a control GUID for a private logger. 
        //
        
        if (TargetGuidProvided != FALSE) {
            LoggerInfo->Wnode.Guid = TargetGuid;
        } else {
            Status = ERROR_INVALID_PARAMETER;
            goto CleanUpAndExit;
        }
    }
    
    //
    // Process the request.
    //

    switch (Action) {
        case  ACTION_START:
        {

            //
            // Use default file name if not given
            //

            if (_tcslen(LogFileName) == 0) {
                _tcscpy_s(LogFileName, MAXSTR, DEFAULT_LOGFILE_NAME); 
            }

            Status = StartTrace(&LoggerHandle, LoggerName, LoggerInfo);

            if (Status != ERROR_SUCCESS) {
                _tprintf(_T("Could not start logger: %s\n") 
                         _T("Operation Status:       %uL\n"),
                         LoggerName,
                         Status);

                break;
            }
            _tprintf(_T("Logger Started...\n"));
        }
        case ACTION_ENABLE:
        {

            //
            // We can allow enabling a GUID during START operation
            // (Note no break in case ACTION_START). 
            // In that case, we do not need to get LoggerHandle separately.
            //

            if (Action == ACTION_ENABLE) {
                
                //
                // Get Logger Handle though Query.
                //

                Status = ControlTrace((TRACEHANDLE)0,
                                      LoggerName,
                                      LoggerInfo,
                                      EVENT_TRACE_CONTROL_QUERY);

                if (Status != ERROR_SUCCESS) {
                    _tprintf(_T("ERROR: Logger not started\n")
                             _T("Operation Status:    %uL\n"),
                             Status);
                    break;
                }
                LoggerHandle = LoggerInfo->Wnode.HistoricalContext;
            }
            
            //
            // We do not allow EnableTrace on the Kernel Logger in this sample,
            // users can use EnableFlags to enable/disable certain kernel events.
            //

            if (IsKernelLogger == FALSE) {
                _tprintf(_T("Enabling trace to logger %d\n"), LoggerHandle);

                //
                // In this sample, we use EnableFlag = EnableLebel = 0.
                //

                Status = EnableTrace(Enable,
                                     0,
                                     0,
                                     &TargetGuid, 
                                     LoggerHandle);

                if (Status != ERROR_SUCCESS) {
                    _tprintf(_T("ERROR: Failed to enable Guid...\n"));
                    _tprintf(_T("Operation Status:       %uL\n"), Status);
                    break;
                }
            }
            break;
        }
        case ACTION_STOP:
        {
            LoggerHandle = (TRACEHANDLE)0;

            Status = ControlTrace(LoggerHandle,
                                  LoggerName,
                                  LoggerInfo,
                                  EVENT_TRACE_CONTROL_STOP);
            break;
        }
        case ACTION_LIST:
        {
            ULONG ReturnCount;
            PEVENT_TRACE_PROPERTIES LoggerInfo[MAXIMUM_LOGGERS];
            PEVENT_TRACE_PROPERTIES Storage;
            PEVENT_TRACE_PROPERTIES TempStorage;
            ULONG SizeForOneProperty = sizeof(EVENT_TRACE_PROPERTIES) +
                                       2 * MAXSTR * sizeof(TCHAR);
            
            //
            // We need to prepare space to receieve the inforamtion for the loggers.
            // Each logger information needs one EVENT_TRACE_PROPERTIES sturucture
            // followed by the logger name and the logfile path strings.
            //

            SizeNeeded = MAXIMUM_LOGGERS * SizeForOneProperty;

            Storage = (PEVENT_TRACE_PROPERTIES)malloc(SizeNeeded);
            if (Storage == NULL) {
                Status = ERROR_OUTOFMEMORY;
                break;
            }

            RtlZeroMemory(Storage, SizeNeeded);

            //
            // Save the pointer for free() later.
            //

            TempStorage = Storage;

	        //
	        // Initialize the LoggerInfo array, before passing it to QueryAllTraces.
	        //

            for (LoggerCounter = 0; LoggerCounter < MAXIMUM_LOGGERS; LoggerCounter++) {

                Storage->Wnode.BufferSize = SizeForOneProperty;
                Storage->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

                Storage->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) +
                                             MAXSTR * sizeof(TCHAR);

                LoggerInfo[LoggerCounter] = Storage;

		    //
		    // Move Storage to point to the next allocated buffer for the
		    // logger information.
		    //

                Storage = (PEVENT_TRACE_PROPERTIES)((PUCHAR)Storage + 
                                                    Storage->Wnode.BufferSize);
            }
        
            Status = QueryAllTraces(LoggerInfo,
                                    MAXIMUM_LOGGERS,
                                    &ReturnCount);
    
            if (Status == ERROR_SUCCESS) {
                for (LoggerCounter = 0; LoggerCounter < ReturnCount; LoggerCounter++) {
                    PrintLoggerStatus(LoggerInfo[LoggerCounter], Status);
                    _tprintf(_T("\n"));
                }
            }

	        //
	        // Free the memory allocated for the logger information buffers.
	        //

            free(TempStorage);
            break;
        }

        case ACTION_UPDATE:
        {

            //
            // In this sample, users can only update MaximumBuffers and log file name. 
            // User can add more options for other parameters as needed.
            //

            Status = ControlTrace(LoggerHandle,
                                  LoggerName,
                                  LoggerInfo,
                                  EVENT_TRACE_CONTROL_UPDATE);
            break;
        }
        case ACTION_QUERY:
        {
            Status = ControlTrace(LoggerHandle,
                                  LoggerName,
                                  LoggerInfo,
                                  EVENT_TRACE_CONTROL_QUERY);
            break;
        }
        case ACTION_HELP:
        {
            PrintHelpMessage();
            break;
        }
        default:
        {
            _tprintf(_T("Error: no action specified\n"));
            PrintHelpMessage();
            break;
        }
    }
    
    if ((Action != ACTION_HELP) && 
        (Action != ACTION_UNDEFINED) && 
        (Action != ACTION_LIST)) {

        PrintLoggerStatus(LoggerInfo, Status);
    }

CleanUpAndExit:

    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
    }

    if (utargv != NULL) {
        GlobalFree(utargv);
    }

    free(LoggerInfo);

    return Status;
}


VOID
PrintLoggerStatus(
    __in PEVENT_TRACE_PROPERTIES LoggerInfo,
    __in ULONG Status
    )

/*++

Routine Description:

    Prints out the status of the specified logger.

Arguments:

    LoggerInfo - The pointer to the resident EVENT_TRACE_PROPERTIES that has
        the information about the current logger.

    Status - The operation status of the current logger.

Return Value:

    None.

--*/

{
    LPTSTR LoggerName;
    LPTSTR LogFileName;
    
    if ((LoggerInfo->LoggerNameOffset > 0) &&
        (LoggerInfo->LoggerNameOffset < LoggerInfo->Wnode.BufferSize)) {

        LoggerName = (LPTSTR)((PUCHAR)LoggerInfo +
                              LoggerInfo->LoggerNameOffset);
    } else {
        LoggerName = NULL;
    }

    if ((LoggerInfo->LogFileNameOffset > 0) &&
        (LoggerInfo->LogFileNameOffset < LoggerInfo->Wnode.BufferSize)) {

        LogFileName = (LPTSTR)((PUCHAR)LoggerInfo +
                               LoggerInfo->LogFileNameOffset);
    } else {
        LogFileName = NULL;
    }

    _tprintf(_T("Operation Status:       %uL\n"), Status);
    
    _tprintf(_T("Logger Name:            %s\n"),
            (LoggerName == NULL) ?
            _T(" ") : LoggerName);

    
    _tprintf(_T("Logger Id:              %I64x\n"), LoggerInfo->Wnode.HistoricalContext);

    _tprintf(_T("Logger Thread Id:       %d\n"), LoggerInfo->LoggerThreadId);

    if (Status != ERROR_SUCCESS) {
        return;
    }

    _tprintf(_T("Buffer Size:            %d Kb"), LoggerInfo->BufferSize);

    if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_PAGED_MEMORY) {
        _tprintf(_T(" using paged memory\n"));
    } else {
        _tprintf(_T("\n"));
    }
    _tprintf(_T("Maximum Buffers:        %d\n"), LoggerInfo->MaximumBuffers);
    _tprintf(_T("Minimum Buffers:        %d\n"), LoggerInfo->MinimumBuffers);
    _tprintf(_T("Number of Buffers:      %d\n"), LoggerInfo->NumberOfBuffers);
    _tprintf(_T("Free Buffers:           %d\n"), LoggerInfo->FreeBuffers);
    _tprintf(_T("Buffers Written:        %d\n"), LoggerInfo->BuffersWritten);
    _tprintf(_T("Events Lost:            %d\n"), LoggerInfo->EventsLost);
    _tprintf(_T("Log Buffers Lost:       %d\n"), LoggerInfo->LogBuffersLost);
    _tprintf(_T("Real Time Buffers Lost: %d\n"), LoggerInfo->RealTimeBuffersLost);
    _tprintf(_T("AgeLimit:               %d\n"), LoggerInfo->AgeLimit);

    if (LogFileName == NULL) {
        _tprintf(_T("Buffering Mode:         "));
    } else {
        _tprintf(_T("Log File Mode:          "));
    }

    if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) {
        _tprintf(_T("Append  "));
    }

    if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
        _tprintf(_T("Circular\n"));
    } else if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) {
        _tprintf(_T("Sequential\n"));
    } else {
        _tprintf(_T("Sequential\n"));
    }

    if (LoggerInfo->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
        _tprintf(_T("Real Time mode enabled"));
        _tprintf(_T("\n"));
    }

    if (LoggerInfo->MaximumFileSize > 0) {
        _tprintf(_T("Maximum File Size:      %d Mb\n"), LoggerInfo->MaximumFileSize);
    }

    if (LoggerInfo->FlushTimer > 0) {
        _tprintf(_T("Buffer Flush Timer:     %d secs\n"), LoggerInfo->FlushTimer);
    }

    if (LoggerInfo->EnableFlags != 0) {
        _tprintf(_T("Enabled tracing:        "));

        if ((LoggerName != NULL) && (_tcscmp(LoggerName, KERNEL_LOGGER_NAME) == 0)) {

            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_PROCESS) {
                _tprintf(_T("Process "));
            }
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_THREAD) {
                _tprintf(_T("Thread "));
            }
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_DISK_IO) {
                _tprintf(_T("Disk "));
            }
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO) {
                _tprintf(_T("File "));
            }
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS) {
                _tprintf(_T("PageFaults "));
            }
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS){
                _tprintf(_T("HardFaults "));
            }
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) {
                _tprintf(_T("ImageLoad "));
            }
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP) {
                _tprintf(_T("TcpIp "));
            }
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_REGISTRY) {
                _tprintf(_T("Registry "));
            }
        } else {
            _tprintf(_T("0x%08x"), LoggerInfo->EnableFlags);
        }

        _tprintf(_T("\n"));
    }

    if (LogFileName != NULL) {
        _tprintf(_T("Log Filename:           %s\n"), LogFileName);
    }

}

ULONG 
HexToLong(
    __in TCHAR *String
    )

/*++

Routine Description:

    Converts a hex string into a number.

Arguments:

    String - A hex string in TCHAR. 

Return Value:

    ULONG - The number in the string.

--*/

{

    ULONG HexDigit = 0;
    ULONG Base = 1;
    ULONG HexValue = 0;
    INT Length = (INT)_tcslen(String);

    while (--Length >= 0) {
        if ((String[Length] == 'x' || String[Length] == 'X') &&
            (String[Length - 1] == '0')) {

            break;
        }

        if ((String[Length] >= '0') && (String[Length] <= '9')) {
            HexDigit = String[Length] - '0';
        } else if ((String[Length] >= 'a') && (String[Length] <= 'f')) {
            HexDigit = (String[Length] - 'a') + 10;
        } else if ((String[Length] >= 'A') && (String[Length] <= 'F')) {
            HexDigit = (String[Length] - 'A') + 10;
        } else {
            continue;
        }

        HexValue |= HexDigit * Base;
        Base <<= 4;
    }

    return HexValue;
}

VOID 
StringToGuid(
    __in TCHAR *String, 
    __out  LPGUID Guid
)

/*++

Routine Description:

    Converts a string into a GUID.

Arguments:

    String - A string in TCHAR.

    Guid - The pointer to a GUID that will have the converted GUID.

Return Value:

    None.

--*/

{
    TCHAR Temp[10];
    INT Index;

    _tcsncpy_s(Temp, 10, String, 8);
    Temp[8] = 0;
    Guid->Data1 = HexToLong(Temp);

    _tcsncpy_s(Temp, 10, &String[9], 4);
    Temp[4] = 0;
    Guid->Data2 = (USHORT)HexToLong(Temp);

    _tcsncpy_s(Temp, 10, &String[14], 4);
    Temp[4] = 0;
    Guid->Data3 = (USHORT)HexToLong(Temp);

    for (Index = 0; Index < 2; Index++) {
        _tcsncpy_s(Temp, 10, &String[19 + Index * 2], 2);
        Temp[2] = 0;
        Guid->Data4[Index] = (UCHAR)HexToLong(Temp);
    }

    for (Index = 2; Index < 8; Index++) {
        _tcsncpy_s(Temp, 10, &String[20 + Index * 2], 2);
        Temp[2] = 0;
        Guid->Data4[Index] = (UCHAR)HexToLong(Temp);
    }
}

VOID
PrintHelpMessage(
    VOID
    )

/*++

Routine Description:

    Prints out a help message.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _tprintf(_T("Usage: tracelog [actions] [options] | [-h | -help | -?]\n"));
    _tprintf(_T("\n    actions:\n"));
    _tprintf(_T("\t-start   [LoggerName] Starts up the [LoggerName] trace session\n"));
    _tprintf(_T("\t-stop    [LoggerName] Stops the [LoggerName] trace session\n"));
    _tprintf(_T("\t-update  [LoggerName] Updates the [LoggerName] trace session\n"));
    _tprintf(_T("\t-enable  [LoggerName] Enables providers for the [LoggerName] session\n"));
    _tprintf(_T("\t-disable [LoggerName] Disables providers for the [LoggerName] session\n"));
    _tprintf(_T("\t-query   [LoggerName] Query status of [LoggerName] trace session\n"));
    _tprintf(_T("\t-list                 List all trace sessions\n"));

    _tprintf(_T("\n    options:\n"));
    _tprintf(_T("\t-um                   Use Process Private tracing\n"));
    _tprintf(_T("\t-max <n>              Sets maximum buffers\n"));
    _tprintf(_T("\t-f <name>             Log to file <name>\n"));
    _tprintf(_T("\t-guid #<guid>         Provider GUID to enable/disable\n"));
    _tprintf(_T("\n"));
    _tprintf(_T("\t-h\n"));
    _tprintf(_T("\t-help\n"));
    _tprintf(_T("\t-?                    Display usage information\n"));
}
