/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    CrashExe.c

Abstract:

    This sample demonstrates how to use the WER runtime exception callbacks.

    This module implements the executable that will register an out-of-process exception handler
    (HandlerDll.dll) and then raise a custom exception code that will be handled by HandlerDll.dll.

--*/

#include <stdio.h>
#include <crtdbg.h>

#include <windows.h>
#include <werapi.h>

#define UNREACHABLE_CODE() _ASSERT(FALSE)

int
wmain (
    int argc,
    const wchar_t* argv[],
    const wchar_t* envp[]
)
{
    HRESULT hr = E_FAIL;
    DWORD rc;
    WCHAR ModulePath[MAX_PATH];
    size_t i;


    UNREFERENCED_PARAMETER (argc);
    UNREFERENCED_PARAMETER (argv);
    UNREFERENCED_PARAMETER (envp);


    //
    // Get the full-path name to ourselves, so we can form a full-path to HandlerDll.dll in this directory:
    //
    //      C:\SomeDirectory\CrashExe.exe
    //                      ^
    //  1) scan until this character (the last forward slash)
    //  2) from that point, replace the substring with "HandlerDll.dll" to form:
    //
    //      C:\SomeDirectory\HandlerDll.dll
    //
    rc = GetModuleFileName (NULL, ModulePath, MAX_PATH);
    if (!rc || (rc == MAX_PATH && ERROR_INSUFFICIENT_BUFFER == GetLastError ())) {
        return 1;
    }

    //
    // Find the last \, so we can replace our exe name with "HandlerDll.dll".
    //
    ModulePath[MAX_PATH-1] = 0;

    for (i = wcslen (ModulePath); i >= 0; --i) {
        if (ModulePath[i] == L'\\') {
            if (0 != wcscpy_s (&ModulePath[i + 1], MAX_PATH - i - 1, L"HandlerDll.dll")) {
                return 1;
            }

            break;
        }
    }

    //
    // Tell WER to register a custom runtime exception module for this process.
    //
    // IMPORTANT: The handler also has to be registered in the system registry by creating a special value
    //            under the HKEY_LOCAL_MACHINE hive. Please refer to 'Running the Sample' section in README.TXT for
    //            complete details.
    //
    wprintf (L"Registering %s as the custom exception handler.\n", ModulePath);

    hr = WerRegisterRuntimeExceptionModule (ModulePath, NULL);

    if (FAILED(hr)) {
        wprintf (L"Failed to register a custom exception handler: 0x%08X\n", hr);
        goto End;
    }

    //
    // Raise a custom exception. This should be handled by HandlerDll.dll.
    //
    // Note that the system will clear bit 28 of dwExceptionCode. This bit is a reserved exception bit,
    // used by the system for its own purposes.
    //
    // The exception we will be using through-out the example is 0xABCD1234 - its bit 28 is clear.
    //
    wprintf (L"Raising exception 0xABCD1234...\n");

    RaiseException (0xABCD1234, EXCEPTION_NONCONTINUABLE, 0, NULL);


    UNREACHABLE_CODE ();

    hr = S_OK;


End:

    return SUCCEEDED(hr) ? 0 : (int)hr;
}
