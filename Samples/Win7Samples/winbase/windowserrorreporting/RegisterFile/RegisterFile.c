/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    RegisterFile.c

Abstract:

    This sample demonstrates how to use the WerRegisterFile API.

    The WerRegisterFile API allows an application to include a custom file that is collected when WER
    creates an error report in the event that an application crashes or hangs.

    In this example, we'll create a mini log file and register it for collection by WER. The contents
    of the registered file are picked up at the time of the crash, not at the time of registration.

--*/

#include <stdio.h>

#include <windows.h>
#include <werapi.h>

int
wmain (
    int argc,
    const wchar_t* argv[],
    const wchar_t* envp[]
)
{
    HRESULT hr = E_FAIL;
    BOOL rc;
    HANDLE LogFileHandle = INVALID_HANDLE_VALUE;
    PCSTR BytesToPrint = NULL;
    DWORD BytesWritten;
    WCHAR LogFullPath[MAX_PATH];


    UNREFERENCED_PARAMETER (argc);
    UNREFERENCED_PARAMETER (argv);
    UNREFERENCED_PARAMETER (envp);


    //
    // Create a log file in the current directory.
    // Make sure we share read access so WER can read the file.
    //
    LogFileHandle = CreateFile (L"mylogfile.txt",
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_ALWAYS,
                                0,
                                NULL);

    if (INVALID_HANDLE_VALUE == LogFileHandle) {
        wprintf (L"Failed to create a logfile: %u (Win32 error code)\n", GetLastError ());
        return -1;
    }

    //
    // Print a few lines into the log file.
    //
    BytesToPrint = "Line 1\nLine 2\n";

    rc = WriteFile (LogFileHandle, BytesToPrint, strlen (BytesToPrint), &BytesWritten, NULL);

    if (!rc) {
        wprintf (L"Error writing to logfile: %u (Win32 error code)\n", GetLastError ());
        return -1;
    }

    //
    // Make sure we flush the log file so the bytes actually make it to the file-system.
    //
    rc = FlushFileBuffers (LogFileHandle);

    if (!rc) {
        wprintf (L"Error flushing the logfile: %u (Win32 error code)\n", GetLastError ());
        return -1;
    }

    //
    // Get the full path to the log file. We need this because WerRegisterFile requires a full path.
    //
    GetFullPathName (L"mylogfile.txt", MAX_PATH, LogFullPath, NULL);

    //
    // Finally, tell WER to collect the file when we crash.
    // Specify that the file does not contain any personally identifiable information, and the file
    // can be safely sent without the corresponding consent from the user.
    //
    wprintf (L"Registering file %s for collection.\n", LogFullPath);

    hr = WerRegisterFile (LogFullPath, WerRegFileTypeOther, WER_FILE_ANONYMOUS_DATA);

    if (FAILED (hr)) {
        wprintf (L"WerRegisterFile has failed: 0x%08X\n", hr);
        return -1;
    }

    //
    // We won't be closing the log file here as it is not an expected act in exceptional circumstances,
    // such as an application crash.
    //

    //
    // Crash this application by writing to a NULL pointer.
    //
    wprintf (L"Crashing the application...\n");
    fflush (stdout);

    *((int*)NULL) = 0;

    return 0;
}
