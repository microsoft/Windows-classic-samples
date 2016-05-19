/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    HandlerDll.c

Abstract:

    This sample demonstrates how to use the WER runtime exception callbacks.

    This module implements the runtime exception module DLL that handles out-of-process exceptions.

--*/

#include <stdio.h>
#include <crtdbg.h>

#include <windows.h>
#include <werapi.h>

#define UNREACHABLE_CODE() _ASSERT(FALSE)

HRESULT WINAPI
OutOfProcessExceptionEventCallback (
  /* __in    */ PVOID pContext,
  /* __in    */ const PWER_RUNTIME_EXCEPTION_INFORMATION pExceptionInformation,
  /* __out   */ BOOL *pbOwnershipClaimed,
  /* __out   */ PWSTR pwszEventName,
  /* __inout */ PDWORD pchSize,
  /* __out   */ PDWORD pdwSignatureCount
)

/*++

Routine Description:

    WER calls this function to determine whether the exception handler is claiming the crash.

    We will check if this is an exception we should handle (0xABCD1234).

    The PFN_WER_RUNTIME_EXCEPTION_EVENT type defines a pointer to this callback function.

    This function will be exported out of this DLL, as specified by HandlerDll.def.

Arguments:

    pContext - An arbitrary pointer-sized value that was passed in to WerRegisterRuntimeExceptionModule.

    pExceptionInformation - A WER_RUNTIME_EXCEPTION_INFORMATION structure that contains the exception
        information. Use the information to determine whether you want to claim the crash.

    pbOwnershipClaimed - Set to TRUE if you are claiming the crash; otherwise, FALSE. If you return FALSE,
        do not set the rest of the out parameters.

    pwszEventName - A caller-allocated buffer that you use to specify the event name used to identify this
        crash.

    pchSize - A pointer to a DWORD specifying the size, in characters, of the pwszEventName buffer. The size
        includes the null-terminating character.

    pdwSignatureCount - The number of report parameters that you will provide. The valid range of values is
        one to 10. If you specify a value greater than 10, WER will ignore the value and collect only the
        first 10 parameters. If you specify zero, the reporting process will be indeterminate.

        This value determines the number of times that WER calls your OutOfProcessExceptionEventSignature-
        Callback function.

Return Value:

    HRESULT.

--*/

{
    PCWSTR EventName = L"MySampleEventName";
    DWORD EventNameLength;


    UNREFERENCED_PARAMETER (pContext);


    //
    // Bail out if it is not an exception we want to handle.
    //
    if (0xABCD1234 != pExceptionInformation->exceptionRecord.ExceptionCode) {
        *pbOwnershipClaimed = FALSE;
        return S_OK;
    }

    //
    // Claim the exception. We will use 2 signature pairs to uniquely identify our exception.
    //
    *pbOwnershipClaimed = TRUE;
    *pdwSignatureCount = 2;

    //
    // See if the event name buffer given is big enough to hold the event name, and the null-terminator.
    //
    EventNameLength = (DWORD) (1 + wcslen (EventName));

    if (*pchSize < EventNameLength) {
        *pchSize = EventNameLength;
        return HRESULT_FROM_WIN32 (ERROR_INSUFFICIENT_BUFFER);
    }

    //
    // Copy the event name we use.
    //
    wcscpy_s (pwszEventName, *pchSize, EventName);

    return S_OK;
}

HRESULT WINAPI
OutOfProcessExceptionEventSignatureCallback (
  /* __in    */ PVOID pContext,
  /* __in    */ const PWER_RUNTIME_EXCEPTION_INFORMATION pExceptionInformation,
  /* __in    */ DWORD dwIndex,
  /* __out   */ PWSTR pwszName,
  /* __inout */ PDWORD pchName,
  /* __out   */ PWSTR pwszValue,
  /* __inout */ PDWORD pchValue
)

/*++

Routine Description:

    WER can call this function multiple times to get the report parameters that uniquely describe the
    problem.

    The PFN_WER_RUNTIME_EXCEPTION_EVENT_SIGNATURE type defines a pointer to this callback function.

    This function will be exported out of this DLL, as specified by HandlerDll.def.

Arguments:

    pContext - An arbitrary pointer-sized value that was passed in to WerRegisterRuntimeExceptionModule.

    pExceptionInformation - A WER_RUNTIME_EXCEPTION_INFORMATION structure that contains the exception
        information.

    dwIndex - The index of the report parameter. Valid values are 0 to 9.

    pwszName - A caller-allocated buffer that you use to specify the parameter name.

    pchName - A pointer to a DWORD specifying the size, in characters, of the pwszName buffer. The size includes
        the null-terminating character.

    pwszValue - A caller-allocated buffer that you use to specify the parameter value.

    pchValue - A pointer to a DWORD specifying the size, in characters, of the pwszValue buffer. The size includes
        the null-terminating character.

Return Value:

    HRESULT.

--*/

{
    UNREFERENCED_PARAMETER (pContext);


    //
    // Some sanity checks. Our handler only specifies 2 signature pairs.
    //
    if (dwIndex >= 2
        || (0xABCD1234 != pExceptionInformation->exceptionRecord.ExceptionCode)) {

        return E_UNEXPECTED;
    }

    //
    // Make sure the given buffers are large enough to hold our signature name/value pairs.
    // We will need 4 characters (3 characters + 1 null-terminator) for our fixed strings.
    //
    if (*pchName < 4) {
        *pchName = 4;
        return HRESULT_FROM_WIN32 (ERROR_INSUFFICIENT_BUFFER);
    }

    if (*pchValue < 4) {
        *pchValue = 4;
        return HRESULT_FROM_WIN32 (ERROR_INSUFFICIENT_BUFFER);
    }

    //
    // At this point, we should fill in the problem signature with the data from the crashing process.
    //
    // For example, the signature can uniquely identify where the crash happened. If our application runs
    // custom-compiled code, we can let the signature identify what module/class/line/etc the crash
    // happened at. This can be done by exposing an easily-accessible data structure in the process, and
    // reading the structure using ReadProcessMemory.
    //
    // In here, we will simply set the signature to some fixed strings.
    //
    switch (dwIndex) {
      case 0:
        wcscpy_s (pwszName, *pchName, L"one");
        wcscpy_s (pwszValue, *pchValue, L"111");
        break;

      case 1:
        wcscpy_s (pwszName, *pchName, L"two");
        wcscpy_s (pwszValue, *pchValue, L"222");
        break;

      default:
        UNREACHABLE_CODE ();
    }

    return S_OK;
}

HRESULT WINAPI
OutOfProcessExceptionEventDebuggerLaunchCallback (
  /* __in    */ PVOID pContext,
  /* __in    */ const PWER_RUNTIME_EXCEPTION_INFORMATION pExceptionInformation,
  /* __out   */ PBOOL pbIsCustomDebugger,
  /* __out   */ PWSTR pwszDebuggerLaunch,
  /* __inout */ PDWORD pchDebuggerLaunch,
  /* __out   */ PBOOL pbIsDebuggerAutolaunch
)

/*++

Routine Description:

    WER calls this function to let you customize the debugger launch options and launch string.

    The PFN_WER_RUNTIME_EXCEPTION_DEBUGGER_LAUNCH type defines a pointer to this callback function.

    This function will be exported out of this DLL, as specified by HandlerDll.def.

Arguments:

    pContext - An arbitrary pointer-sized value that was passed in to WerRegisterRuntimeExceptionModule.

    pExceptionInformation - A WER_RUNTIME_EXCEPTION_INFORMATION structure that contains the exception
        information.

    pbIsCustomDebugger - Set to TRUE if you want to use the custom debugger specified in the
        pwszDebuggerLaunch parameter; otherwise, FALSE to use the default debugger. If you return FALSE, do
        not set the pwszDebuggerLaunch parameter.

    pwszDebuggerLaunch - A caller-allocated buffer that you use to specify the debugger launch string used to
        launch the debugger.

        The launch string must include the full path to the debugger and any arguments. If an argument
        ncludes multiple words, use quotes to delimit the argument.

    pchDebuggerLaunch - A pointer to a DWORD specifying the size, in characters, of the pwszDebuggerLaunch buffer.
        The size includes the null-terminating character.

    pbIsDebuggerAutolaunch - Set to TRUE if you want WER to silently launch the debugger; otherwise, FALSE if
        you want WER to ask the user before launching the debugger.

Return Value:

    HRESULT.

--*/

{
    UNREFERENCED_PARAMETER (pContext);


    //
    // Some sanity checks.
    //
    if ((0xABCD1234 != pExceptionInformation->exceptionRecord.ExceptionCode)) {
        return E_UNEXPECTED;
    }

    //
    // Specify that we won't be using any custom debugger for this.
    //
    *pbIsCustomDebugger = FALSE;

    return S_OK;
}

BOOL WINAPI
DllMain (
    HINSTANCE DllInstance,
    DWORD Reason,
    LPVOID Reserved
)

{
    UNREFERENCED_PARAMETER (DllInstance);
    UNREFERENCED_PARAMETER (Reason);
    UNREFERENCED_PARAMETER (Reserved);


    return TRUE;
}
