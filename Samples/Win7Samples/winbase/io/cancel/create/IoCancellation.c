//======================================================================
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//======================================================================

/*++

    Module Name:

        IoCancellation.c

    Abstract:

        Implementation of the safe IO cancellation interface
                
--*/

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "IoCancellation.h"

DWORD
IoCancellationCreate(
    PIO_CANCELLATION_OBJECT* ppObject
	)
/*++

Routine Description:

    Creates new cancellation object.

Arguments:

    ppObject - Pointer to receive new cancellation object.

Return values:
 
    ERROR_NOT_SUPPORTED if the OS does not support synchronous cancellation.
	ERROR_SUCCESS if the creation was successful.
	Otherwise an appropriate error code is returned.


--*/
{
    PIO_CANCELLATION_OBJECT pObject   = NULL;
    HANDLE                  hKernel32 = NULL;
    PFN_CancelSynchronousIo pfnCancel = NULL;
    BOOL                    fCS       = FALSE;
    DWORD                   err       = 0;

    //
    // Load CancelSynchronousIo dynamically from kernel32.dll
    //
    hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!hKernel32) {
        err = GetLastError();
        goto cleanup;
    }

    //
    // Return ERROR_NOT_SUPPORTED CancelSynchronousIo not found
    //
    pfnCancel = (PFN_CancelSynchronousIo)
        GetProcAddress(hKernel32, "CancelSynchronousIo");
    if (!pfnCancel) {
        err = GetLastError();
        if (err == ERROR_PROC_NOT_FOUND) {
            err = ERROR_NOT_SUPPORTED;
        }

        goto cleanup;
    }

    //
    // Allocate and initialize the object
    //
    pObject = (PIO_CANCELLATION_OBJECT)
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pObject));
    if (!pObject) {
        err = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    fCS = InitializeCriticalSectionAndSpinCount(&pObject->aCS, 0);
    if (!fCS) {
        err = GetLastError();
        goto cleanup;
    }

    //
    // The hThread member will be intialized on demand
    // by IoCancellationSectionEnter
    //
    pObject->pfnCancelSynchronousIo = pfnCancel;

    //
    // Detach and return
    //
    *ppObject = pObject;
    pObject   = NULL;

cleanup:
    
    if (pObject) {
        if (fCS) {
            DeleteCriticalSection(&pObject->aCS);
        }

        HeapFree(GetProcessHeap(), 0, pObject);
    }

    return err;
}

VOID
IoCancellationClose(
    PIO_CANCELLATION_OBJECT pObject)
/*++

Routine Description:

    Closes a cancellation object.

Arguments:

    pObject - Pointer to cancellation object.

--*/
{
    //
    // Lock used as memory barrier
    //
    EnterCriticalSection(&pObject->aCS);

    assert(!pObject->fPendingIo);

    if (pObject->hThread) {
        CloseHandle(pObject->hThread);
    }

    LeaveCriticalSection(&pObject->aCS);

    DeleteCriticalSection(&pObject->aCS);
    HeapFree(GetProcessHeap(), 0, pObject);
}


DWORD
IoCancellationSectionEnter(
    PIO_CANCELLATION_OBJECT pObject
	)
/*++

Routine Description:

    Used to indicate the start of a portion of code in which a 
	synchronous operation can be cancelled.
	.

Arguments:

    pObject - Pointer to a previously allocated cancellation object.

--*/
{
    DWORD err = 0;

    EnterCriticalSection(&pObject->aCS);

    //
    // Recursive calls are not allowed
    //
    assert(!pObject->fPendingIo);

    //
    // Check if already canceled
    //
    if (pObject->fCanceled) {
        err = ERROR_OPERATION_ABORTED;
        goto cleanup;
    }

    //
    // Replace the thread handle if needed
    //
    if (pObject->hThread && pObject->dwThreadId != GetCurrentThreadId()) {

        CloseHandle(pObject->hThread);
        pObject->hThread    = NULL;
        pObject->dwThreadId = 0;
    }

    //
    // GetCurrentThread returns pseudo handle for the calling thread
    // and it always refers to the current thread; therefore it can't 
	// be used for CancelSynchronousIo.
    // DuplicateHandle will return a normal handle that can
    // be used cross thread.
    //
    if (!pObject->hThread) {
        //
        // CancelSynchronousIo actually requires only THREAD_TERMINATE
        //
        BOOL fOK =  DuplicateHandle(
            GetCurrentProcess(), 
            GetCurrentThread(), 
            GetCurrentProcess(),
            &pObject->hThread, 
            THREAD_ALL_ACCESS,
            FALSE,   // not inherited
            0
        );
        if (!fOK) {
            err = GetLastError();
            goto cleanup;
        }

        pObject->dwThreadId = GetCurrentThreadId();
    }

    assert(pObject->hThread != NULL);
    assert(pObject->dwThreadId == GetCurrentThreadId());

    pObject->fPendingIo = TRUE;

cleanup:
    LeaveCriticalSection(&pObject->aCS);
    return err;
}

DWORD
IoCancellationSectionLeave(
    PIO_CANCELLATION_OBJECT pObject
	)
/*++

Routine Description:

    Used to indicate the end of a portion of code in which a 
	synchronous operation can be cancelled.
	.

Arguments:

    pObject - Pointer to a cancellation object that was used to
	    enter the cancellation section.

Return Value:

	ERROR_OPERATION_ABORTED if the operation had cancellation attempted.
	ERROR_SUCCESS otherwise.

--*/
{
    DWORD err = ERROR_SUCCESS;

    EnterCriticalSection(&pObject->aCS);

    //
    // Leave only allowed upon successful Enter from the same thread
    //
    assert(pObject->hThread != NULL);
    assert(pObject->dwThreadId == GetCurrentThreadId());
    assert(pObject->fPendingIo);
    
    //
    // Clearing the pending flag always succeeds
    //
    pObject->fPendingIo = FALSE;

    if (pObject->fCanceled) {

        err = ERROR_OPERATION_ABORTED;
        goto cleanup;
    }

cleanup:
    LeaveCriticalSection(&pObject->aCS);
    return err;
}

//
// IoCancellationSignal
//
VOID
IoCancellationSignal(
    PIO_CANCELLATION_OBJECT pObject
	)
/*++

Routine Description:

    This routine will attempt to cancel the operation that
	is represented by the cancellation object by calling
	CancelSynchronousIo on the target thread.

Arguments:

    pObject - Pointer to a cancellation object.

--*/
{
    DWORD dwRetryCount = 0;

    EnterCriticalSection(&pObject->aCS);

    pObject->fCanceled = TRUE;

    //
    // Retry 3 times in case that the IO has not yet made it to
	// the driver and CancelSynchronousIo returns ERROR_NOT_FOUND
    //
    for (dwRetryCount = 0 ; dwRetryCount < 3 ; ++dwRetryCount)
    {
        BOOL fOK = FALSE;

        if (!pObject->fPendingIo) {
            break;
        }

        assert(pObject->hThread != NULL);

        //
        // Ignore cancel errors since it's only optional for drivers
        //
        fOK = pObject->pfnCancelSynchronousIo(pObject->hThread);        

        if (fOK || GetLastError() != ERROR_NOT_FOUND) {
            break;
        }

        //
        // There is small window in which CancelSynchronousIo can be called before (or after) 
        // the actual IO operation and in this case we can retry to cancel after a short delay 
        // in which it's probable that we'll not miss the operation.
        //

        LeaveCriticalSection(&pObject->aCS);

        //
        // Retry after short sleep
        //
        SwitchToThread();

        EnterCriticalSection(&pObject->aCS);
    }

    LeaveCriticalSection(&pObject->aCS);
}

DWORD
CancelableCreateFileW(

    HANDLE*                 phFile,
    PIO_CANCELLATION_OBJECT pCancellationObject,
    LPCWSTR                 wszFileName,
    DWORD                   dwDesiredAccess,
    DWORD                   dwShareMode,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD                   dwCreationDisposition,
    DWORD                   dwFlagsAndAttributes,
    HANDLE                  hTemplateFile
	)
/*++

Routine Description:

    Function that calls CreateFileW in a cancellation section and therefore 
	can be canceled using IoCancellationSignal from another thread.

Arguments:

	phFile - Pointer to a handle that will be returned on successful create.
	    On failure this will be INVALID_HANDLE_VALUE.


    pCancellationObject - Pointer to cancellation object created with 
	    IoCancellationCreate.

    wszFileName - File name to open.

    dwDesiredAccess - Desired access for handle.

    dwShareMode - Share mode.

    lpSecurityAttributes - Optional pointer to security attributes.

    dwCreationDisposition - Disposition for create.

    dwFlagsAndAttributes - Flags for create.

    hTemplateFile - Optional handle to template file.

Return value:

    ERROR_SUCCESS on success.
    ERROR_OPERATION_ABORTED on cancellation.
    Otherwise use GetLastError to determine the cause of failure.

--*/
{
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    DWORD   err   = 0;

    *phFile = INVALID_HANDLE_VALUE;

    err = IoCancellationSectionEnter(pCancellationObject);
    if (err) {
        goto cleanup;
    }

    //
    // Cancellation section contains a single call to CreateFileW
    //
    hFile = CreateFileW(
        wszFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        err = GetLastError();
    }

    //
    // Leave must be called after successful Enter
    //
    err = IoCancellationSectionLeave(pCancellationObject);
    if (err) {
        goto cleanup;
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        goto cleanup;
    }

    *phFile = hFile;
    hFile   = INVALID_HANDLE_VALUE;

cleanup:

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return err;
}


