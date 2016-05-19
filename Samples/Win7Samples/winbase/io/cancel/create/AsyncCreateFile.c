//======================================================================
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//======================================================================

/*++

    Abstract:

    This file contains an application for the IO cancellation object.
    The application is an asynchronous create file that supports cancellation.

    This file also demonstrate the usage for this function in a console 
    application that can check the existence of a file with a timeout.
    
    This sample code is written in a platform independent way and it should 
	also work on versions of Windows without full cancellation support.
                
--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "IoCancellation.h"

//
// ASYNC_CREATE_FILE_CONTEXT
//
typedef struct _ASYNC_CREATE_FILE_CONTEXT
{
    //
    // Handle to thread create to call CreateFile
    //
    HANDLE                      hThread;
    
    //
    // CreateFile status
    //
    DWORD                       dwStatus;
    
    //
    // Handle returned by CreateFile
    //
    HANDLE                      hFile;
    
    //
    // Cancellation Object
    //
    PIO_CANCELLATION_OBJECT     pCancellationObject;
    
    //
    // Arguments to CreateFile call
    //
    LPCWSTR                     wszFileName;
    DWORD                       dwDesiredAccess;
    DWORD                       dwShareMode;
    LPSECURITY_ATTRIBUTES       lpSecurityAttributes;
    DWORD                       dwCreationDisposition;
    DWORD                       dwFlagsAndAttributes;
    HANDLE                      hTemplateFile;
} ASYNC_CREATE_FILE_CONTEXT, *PASYNC_CREATE_FILE_CONTEXT;

DWORD
AsyncCreateFile(
    PASYNC_CREATE_FILE_CONTEXT* ppObject,
    LPCWSTR                     wszFileName,
    DWORD                       dwDesiredAccess,
    DWORD                       dwShareMode,
    LPSECURITY_ATTRIBUTES       lpSecurityAttributes,
    DWORD                       dwCreationDisposition,
    DWORD                       dwFlagsAndAttributes,
    HANDLE                      hTemplateFile
);

VOID
CancelAsyncCreateFile(
    PASYNC_CREATE_FILE_CONTEXT  pObject
);

VOID
CleanupAsyncCreateFile(
    PASYNC_CREATE_FILE_CONTEXT  pObject
);


//
// main
//
int __cdecl
wmain(int argc, wchar_t* argv[])
{
    PASYNC_CREATE_FILE_CONTEXT pContext     = NULL;
    LPCWSTR                    wszFileName  = NULL;
    DWORD                      dwWaitStatus = 0;
    DWORD                      dwTimeout    = 0;
    DWORD                      err          = 0;
	BOOL                       bReturnOnTimeout = FALSE;

    if (argc < 4) {
        printf("Usage %ls [file name] [timeout in milliseconds] [return on timeout - 0/1]\n", argv[0]);
        goto cleanup;
    }

    //
    // Get the command line arguments
    //
    wszFileName = argv[1];
    dwTimeout   = (DWORD)_wtoi(argv[2]);
	bReturnOnTimeout = (DWORD)_wtoi(argv[3]);

    //
    // Call CreateFile to check the existence of a file
    // 
    err = AsyncCreateFile( &pContext,
		                   wszFileName,
						   GENERIC_READ,
						   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL,
						   NULL );

    if (!err) {

        printf("File found: %ls\n", wszFileName);
        goto cleanup;

	} else if (err != ERROR_IO_PENDING) {

       goto cleanup;
    }

	err = 0;

    //
    // Wait with timeout on the worker thread
    //
    dwWaitStatus = WaitForSingleObject(pContext->hThread, dwTimeout);

    switch (dwWaitStatus) {

    case WAIT_OBJECT_0: 

		//
        // Check the status after CreateFile completed
        //
        if (pContext->dwStatus != ERROR_SUCCESS) {

            err = pContext->dwStatus;

		} else {

			printf("File found: %ls\n", wszFileName);
		}

        break;
    
    case WAIT_TIMEOUT: 

		//
		// If we timeout, attempt to cancel the create.  Note that
		// this may not do anything depending on the current status of
		// the operation
		//
		
        CancelAsyncCreateFile(pContext);

		//
		// If the caller specified we should wait after the cancel call for
		// the create to complete, we do that here; otherwise we return
		// an error.
		//
		
		if (bReturnOnTimeout == TRUE) {

			err = ERROR_OPERATION_ABORTED;

		} else {

			dwWaitStatus = WaitForSingleObject(pContext->hThread, INFINITE);
			err = pContext->dwStatus;
		}

        break;

    default:
        
        err = GetLastError();
        break;
    }

cleanup:
    if (err) {

        if (err == ERROR_OPERATION_ABORTED) {

			if (bReturnOnTimeout == TRUE) {

				printf("Operation timed out while trying to open %ls\n", wszFileName);

			} else {

				//
				// If we waited for the create to complete and we still got 
				// this error code, we know that it was successfully cancelled.
				//
				printf("Operation timed out and was cancelled while trying to open %ls\n", wszFileName);
			}

        } else {

            printf("Error %u while trying to open %ls\n", err, wszFileName);
        }
    }

    if (pContext) {
        CleanupAsyncCreateFile(pContext);
    }

    return err ? 2 : 0;
}

DWORD WINAPI
AsyncCreateFileCallback (
    LPVOID pParameter
	)
/*++

Routine Description:

	This is a worker routine that actually issues the asynchronous create.

Arguments:

	pParameter - Pointer to async create context.

Return Value:

	Always 0.

--*/
{
    PASYNC_CREATE_FILE_CONTEXT pContext = NULL;
    DWORD                      err      = 0;

    pContext = (PASYNC_CREATE_FILE_CONTEXT)pParameter;

    err = CancelableCreateFileW( &pContext->hFile,
		                         pContext->pCancellationObject,
								 pContext->wszFileName,
								 pContext->dwDesiredAccess,
								 pContext->dwShareMode,
								 pContext->lpSecurityAttributes,
								 pContext->dwCreationDisposition,
								 pContext->dwFlagsAndAttributes,
								 pContext->hTemplateFile );

    if (err) {
        //
        // Ensure we propagate the status back
		//
        pContext->dwStatus = err;
        goto cleanup;
    }

cleanup:
    return 0;
}

DWORD
AsyncCreateFile(
    PASYNC_CREATE_FILE_CONTEXT* ppObject,
    LPCWSTR                     wszFileName,
    DWORD                       dwDesiredAccess,
    DWORD                       dwShareMode,
    LPSECURITY_ATTRIBUTES       lpSecurityAttributes,
    DWORD                       dwCreationDisposition,
    DWORD                       dwFlagsAndAttributes,
    HANDLE                      hTemplateFile)
/*++

Routine Description:

    This function will attempt to create a thread in order to initiate
	a create request that can be cancelled at a later point in time
	using CancelAsyncCreateFile.

	If the OS does not support cancellation (pre-Vista), the create will
	take place synchronously.

Arguments:

	ppObject - This parameter will receive a pointer to an async create context
	    object that represents this instance of the asynchronous create.  If
	    this is NULL the create was not successful.  In the success case the
	    file handle can be found in this object.

    wszFileName - Target file name to pass to CreateFile call.  Caller needs
	    to ensure this memory is valid until the create completes.

	dwDesiredAccess - Desired Access to pass to CreateFile call.

	dwShareMode - Share mode to pass to CreateFile call.

	lpSecurityAttributes - Security Attributes to pass to CreateFile call.

	dwCreationDisposition - Creation Disposition to pass to CreateFile call.

	dwFlagsAndAttributes - Flags and Attributes to pass to CreateFile call.

	hTemplateFile - Template File handle to pass to CreateFile call.

Return Value:

    ERROR_SUCCESS - The create was successfully completed synchronously.

	ERROR_IO_PENDING - The create was successfully issued asynchronously.
	    The caller should wait for the thread to exit at which point
		they can retrieve the status of the create from ppObject.

	Other failures may happen with an appropriate error code (such as 
	ERROR_OUTOFMEMORY).

--*/
{
    PASYNC_CREATE_FILE_CONTEXT  pObject = NULL;
    DWORD                       err     = 0;

    *ppObject = NULL;

    pObject = (PASYNC_CREATE_FILE_CONTEXT)
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pObject));

    if (pObject == NULL) {
        err = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    pObject->hFile                  = INVALID_HANDLE_VALUE;
	pObject->hThread                = INVALID_HANDLE_VALUE;
    pObject->wszFileName            = wszFileName;
    pObject->dwDesiredAccess        = dwDesiredAccess;
    pObject->dwShareMode            = dwShareMode;
    pObject->lpSecurityAttributes   = lpSecurityAttributes;
    pObject->dwCreationDisposition  = dwCreationDisposition;
    pObject->dwFlagsAndAttributes   = dwFlagsAndAttributes;
    pObject->hTemplateFile          = hTemplateFile;

    err = IoCancellationCreate(&pObject->pCancellationObject);

    if (err == ERROR_NOT_SUPPORTED) {

        pObject->pCancellationObject = NULL;
        err = 0;
    }

    if (err) {
        goto cleanup;
    }

	//
    // Call CreateFile asynchronously if cancellation is applicable
	//
    if (pObject->pCancellationObject) {

        pObject->hThread = CreateThread( NULL,   // default security
                                         0,      // default stack size
										 AsyncCreateFileCallback,
										 pObject,
										 0,      // default creation
										 NULL ); // no need for thread id

        if (pObject->hThread == INVALID_HANDLE_VALUE) {

            err = GetLastError();
            goto cleanup;
        }

        // not a failure
        err = ERROR_IO_PENDING;

    } else {

		//
        // Call CreateFile synchronously if cancellation is not applicable
		//
        pObject->hFile = CreateFileW( pObject->wszFileName,
			                          pObject->dwDesiredAccess,
									  pObject->dwShareMode,
									  pObject->lpSecurityAttributes,
									  pObject->dwCreationDisposition,
									  pObject->dwFlagsAndAttributes,
									  pObject->hTemplateFile );

		if (pObject->hFile == INVALID_HANDLE_VALUE) {
            err = GetLastError();
        }

        if (err) {
            goto cleanup;
        }
    }

    // Detach and retrun
    *ppObject = pObject;
    pObject   = NULL;

cleanup:
    
    if (pObject) {
        CleanupAsyncCreateFile(pObject);
    }

    return err;
}

VOID
CancelAsyncCreateFile(
    PASYNC_CREATE_FILE_CONTEXT  pObject
	)
/*++

Routine Description:

	Attemps to cancel an asynchronous CreateFile.

	This function can be called if AsyncCreateFile returned ERROR_IO_PENDING.

	After calling this function, the called should wait on the thread
	in the object to determine when the create has actually returned (since
	there is no guarantee it will successfully be cancelled) and be sure
	to clean up using CleanupAsyncCreateFile.

Arguments:

	pObject - Pointer to a context object returned by AsyncCreateFile.

--*/
{
	IoCancellationSignal(pObject->pCancellationObject);	
}

VOID
CleanupAsyncCreateFile(
    PASYNC_CREATE_FILE_CONTEXT  pObject
	)
/*++

Routine Description:

	Cleans up the context created to support AsyncCreateFile.

	Callers should use this routine after a call to AsyncCreateFile
	has returned ERROR_SUCCESS or ERROR_IO_PENDING and they are
	finished with the result of the create (for instance on a successful
	create they no longer need the file handle).
	
Arguments:

	pObject - Pointer to a context object returned by AsyncCreateFile.

--*/
{
    if (pObject->hThread != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(pObject->hThread, INFINITE);
        CloseHandle(pObject->hThread);
    }

	//
	// Once the thread has exited we can safely assume that we
	// have full control of the object.
	//
    if (pObject->pCancellationObject != NULL) {
        IoCancellationClose(pObject->pCancellationObject);
    }
  
    if (pObject->hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(pObject->hFile);
    }

    HeapFree(GetProcessHeap(), 0, pObject);
}

