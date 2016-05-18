/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    AppRestart.c

Abstract:

    This sample demonstrates how to use the RegisterApplicationRestart API.

    The RegisterApplicationRestart API lets Windows Error Reporting restart the application automatically
    in the event that the application crashes or hangs, or has to be restarted due to an update.

    In this sample, we register the application for restart. We tell WER to restart the application with the
    "/restarted" command-line. This way the application can do custom processing when it is restarted.

    A process is eligible for automatic restart only if it had been running for longer than 60 seconds.
    This is done to prevent cyclical restarts.

    You may also choose to combine automatic application restart with application recovery, as shown in the
    AppRecovery sample.

--*/

#include <stdio.h>

#include <windows.h>

int
wmain (
    int argc,
    const wchar_t* argv[],
    const wchar_t* envp[]
)
{
    HRESULT hr = E_FAIL;
    int i;


    UNREFERENCED_PARAMETER (envp);


    //
    // Have we been launched as part of a restart?
    //
    if (argc >= 2 &&
        0 == _wcsicmp (argv[1], L"/restarted")) {

        wprintf (L"The application has been restarted.\n");
        wprintf (L"Press ENTER to exit.\n");
        getwc (stdin);
        return 0;
    }

    //
    // Otherwise, we are being run normally.
    //

    //
    // Register the application for restart.
    //
    hr = RegisterApplicationRestart (L"/restarted",
                                     0);

    if (FAILED (hr)) {
        wprintf (L"RegisterApplicationRestart failed with 0x%08X\n", hr);
        return -1;
    }

    wprintf (L"Successfully registered this process for restart.\n");

    //
    // Wait at least 60 seconds before crashing, so this process becomes eligible for restart.
    //
    wprintf (L"Waiting 62 seconds...");

    for (i = 0; i < 62; ++i) {
        wprintf (L" %d", i);

        Sleep (1000);
    }

    wprintf (L"\n");

    //
    // Crash the application by writing to a NULL pointer.
    //
    wprintf (L"Crashing the application...\n");
    fflush (stdout);

    *((int*)NULL) = 0;


    return 0;
}
