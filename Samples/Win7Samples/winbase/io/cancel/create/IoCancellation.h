//======================================================================
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//======================================================================

#pragma once

/*++

    Module Name:

        IoCancellation.h

    Abstract:

        IO cancellation facility that provides safe coding pattern for 
        cancellation of synchronous IO operations using CancelSynchronousIo.
        
        This facility allows to cancel synchronous IO during an IO 
        cancelable code section that is bounded using
        IoCancellationSectionEnter and IoCancellationSectionLeave
        
        In the cancelable section you should call only operations that 
        are cancellation safe (e.g. CreateFile) and can be canceled a 
        using single call to IoCancellationSignal.
        
        Using CancelSynchronousIo without proper synchronization can 
        lead to unpredictable results since CancelSynchronousIo 
        can accidentally cancel the wrong operation.

		See README.TXT and the comments in IoCancellation.c for
		descriptions of the various routines.

--*/

#ifdef __cplusplus
extern "C" {
#endif
	
//
// FN_CancelSynchronousIo
// Prototype from Win32 API - CancelSynchronousIo
//
typedef
BOOL WINAPI
FN_CancelSynchronousIo(
    HANDLE hThread
);

typedef FN_CancelSynchronousIo* PFN_CancelSynchronousIo;

typedef struct _IO_CANCELLATION_OBJECT
{
    CRITICAL_SECTION        aCS;
    HANDLE                  hThread;
    DWORD                   dwThreadId;
    BOOL                    fPendingIo;
    BOOL                    fCanceled;
    PFN_CancelSynchronousIo pfnCancelSynchronousIo;
} IO_CANCELLATION_OBJECT, *PIO_CANCELLATION_OBJECT;

DWORD
IoCancellationCreate(
    PIO_CANCELLATION_OBJECT* ppObject
);

VOID
IoCancellationClose(
    PIO_CANCELLATION_OBJECT pObject
);

DWORD
IoCancellationSectionEnter(
    PIO_CANCELLATION_OBJECT pObject
);

DWORD
IoCancellationSectionLeave(
    PIO_CANCELLATION_OBJECT pObject
);

VOID
IoCancellationSignal(
    PIO_CANCELLATION_OBJECT pObject
);

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
);

#ifdef __cplusplus
} // extern "C"
#endif


