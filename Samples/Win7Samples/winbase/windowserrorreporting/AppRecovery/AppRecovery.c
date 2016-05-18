/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    AppRecovery.c

Abstract:

    This sample demonstrates how to use the RegisterApplicationRecoveryCallback API.

    The RegisterApplicationRecoveryCallback API registers a special callback that Windows Error Reporting will
    call prior to terminating the process due to a crash or a hang.

    The callback is run in context of the faulting process. The application should use this callback to save
    any volatile data or state information to disk or any other non-volatile storage where it can restore the
    data from on next restart.

    You may also choose to combine application recovery with automatic application restart, as shown in the
    AppRestart sample.

--*/

#include <stdio.h>

#include <windows.h>

//
// This is the volatile state information we will be saving to disk during recovery.
//
typedef struct _STATE_BLOCK {
    char RandomNumber[16];
} STATE_BLOCK, *PSTATE_BLOCK;

static STATE_BLOCK g_StateBlock;

//
// This is the recovery callback called by WER.
//
static
DWORD WINAPI
MyRecoveryCallback (
    PVOID pvParameter
)
{
    HRESULT hr = E_FAIL;
    BOOL rc;
    HANDLE FileHandle;
    DWORD BytesWritten;
    BOOL RecoveryCancelled;
    PSTATE_BLOCK StateBlock = (PSTATE_BLOCK) pvParameter;


    //
    // Let WER know that we are recovering. In return, WER will let us know if the user has cancelled recovery.
    //
    hr = ApplicationRecoveryInProgress (&RecoveryCancelled);
    if (FAILED (hr)) {
        //
        // Something bad has happened... bail out of recovery.
        //
        ApplicationRecoveryFinished (FALSE);

        return 0;
    }

    if (RecoveryCancelled) {
        //
        // The user has cancelled recovery.
        //

        return 0;
    }

    //
    // Do the recovery here. Should the following take longer than RECOVERY_DEFAULT_PING_INTERVAL,
    // we would need to call ApplicationRecoveryInProgress to let WER know we are still recovering.
    // Otherwise, WER will terminate recovery and the process.
    //
    // We will do the bare minimum to save the state block to disk. We won't create any new threads or
    // load any DLLs, as the process state may be too corrupted.
    //
    FileHandle = CreateFile (L"recovered_data.txt",
                             GENERIC_WRITE,
                             0,
                             NULL,
                             CREATE_ALWAYS,
                             0,
                             NULL);

    if (INVALID_HANDLE_VALUE != FileHandle) {
        //
        // Write the whole state block to the file.
        //
        rc = WriteFile (FileHandle, StateBlock, sizeof (STATE_BLOCK), &BytesWritten, NULL);

        if (!rc) {
            //
            // We couldn't write to the file. Let WER know we are done with recovery, but it had failed.
            //
            ApplicationRecoveryFinished (FALSE);

            return 0;
        }

        CloseHandle (FileHandle);

        //
        // For effect's sake, wait a few seconds so the recovery state is visible in the WER UI flow.
        // You shouldn't normally do this or take more time for recovery than needed.
        //
        Sleep (3500);

        //
        // Let WER know that we successfully completed recovery.
        //
        ApplicationRecoveryFinished (TRUE);

        return 0;
    }
    else {
        //
        // We couldn't open the file. Let WER know we are done with recovery, but it had failed.
        //
        ApplicationRecoveryFinished (FALSE);

        return 0;
    }
}

int
wmain (
    int argc,
    const wchar_t* argv[],
    const wchar_t* envp[]
)
{
    HRESULT hr = E_FAIL;


    UNREFERENCED_PARAMETER (argc);
    UNREFERENCED_PARAMETER (argv);
    UNREFERENCED_PARAMETER (envp);


    //
    // Register the application for recovery.
    //
    hr = RegisterApplicationRecoveryCallback (MyRecoveryCallback,
                                              &g_StateBlock,
                                              RECOVERY_DEFAULT_PING_INTERVAL,
                                              0);

    if (FAILED (hr)) {
        wprintf (L"RegisterApplicationRecoveryCallback failed with 0x%08X\n", hr);
        return -1;
    }

    wprintf (L"Successfully registered this process for recovery.\n");

    //
    // Save a random number into our state block. This is the block we will be trying to recover later.
    //
    sprintf_s (g_StateBlock.RandomNumber, sizeof (g_StateBlock.RandomNumber), "%d", rand ());

    //
    // Crash the application by writing to a NULL pointer.
    //
    wprintf (L"Crashing the application...\n");
    fflush (stdout);

    *((int*)NULL) = 0;


    return 0;
}
