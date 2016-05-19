// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: asyncselect.cpp
//
// Description:
//    This file contains the routines necessary to implement the WSPAsyncSelect
//    API. In order for our LSP to count the bytes sent and received by an 
//    application when the app uses WSAAsyncSelect we need to intercept all
//    of the app's send and receive calls. To do this we implement a hidden
//    window. When the app makes a send/recv call we intercept this and post
//    the operation with the provider's socket to our own window. This allows
//    the LSP to receive the completion notification. At this point we will
//    update the statistics and then post the completion to the app's window
//    so that it may continue as expected.
//
//    Note that any LSP which uses WPUCreateSocketHandle to return socket handles
//    to the upper layer must intercept all WSAAsyncSelect calls to use this
//    hidden window method. This is required since the wParam parameter passed
//    into the window's async handler is the socket handle on which the event
//    occured. If the LSP doesn't capture this then the wrong socket handle 
//    will be passed directly to the upper layer's window handler.
//
//    This file contains the I/O manager for all WSAAsyncselect I/O operations
//
#include "lspdef.h"
#include <windows.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

//
// We must register a window class with a unique name. To do so we'll
// use PROVIDER_CLASS string to format one which includes the current
// process ID. This string will be formatted into a buffer of 
// PROVIDER_CLASS_CHAR_LEN length.
//
#define PROVIDER_CLASS          TEXT("Layered WS2 Provider 0x%08x")
#define PROVIDER_CLASS_CHAR_LEN 32

//
// When handing window messages it is possible we'll get a message for a
// socket not in our list of created sockets. This can occur on an accepted
// socket since some messages may be posted to the accepted socket before
// the SOCK_INFO structure is inserted into the socket list in WSPAccept.
// So if we don't find a SOCK_INFO structure, we'll sleep and try again
// (up to MAX_ASYNC_RETRIES).
//
#define MAX_ASYNC_RETRIES       7
#define ASYNC_TIMEOUT           500     // half a second

#pragma warning(disable:4127)       // Disable conditional expression is constant warning
#pragma warning(disable:4706)       // Disable assignment within conditional warning (message pump)

//
// Function Prototypes
//

// Creates the hidden window and is the message pump for it
static DWORD WINAPI 
AsyncMsgHandler(
    LPVOID  lpParameter
    );

// Handler function for our Winsock FD_* events which posts completions to upper layer
static LRESULT CALLBACK 
AsyncWndProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    );

//
// Globals to this file
//
static HANDLE WorkerThreadHandle = NULL;        // Dispatch thread for handing window messages
static HWND   AsyncWindow = NULL;               // Handle to the hidden window
static TCHAR  AsyncProviderClassName[ PROVIDER_CLASS_CHAR_LEN ];

//
// Function: StopAsyncWindowManager
//
// Description:
//    This function cleans up the subsystem that handles asynchronous
//    window IO (e.g. WSAAsyncSelect). Basically, it destroys the the
//    hidden window if its been created. Note that this function is
//    only called from WSPCleanup when the DLL is about to be unloaded,
//    and we've already entered the critical section (gCriticalSection).
//
int 
StopAsyncWindowManager(
    )
{
    int     rc,
            code;

    if ( NULL != AsyncWindow )
    {
        // Post a quit message to the thread
        PostMessage( AsyncWindow, WM_DESTROY, 0, 0 );

        // Wait for the thread to cleanup and exit
        rc = WaitForSingleObject( WorkerThreadHandle, 10000 );
        if ( WAIT_TIMEOUT == rc )
        {
            dbgprint("StopAsyncWindowManager: Timed out waiting for async thread!");
            goto cleanup;
        }

        // Retrieve the exit code and display a simple message
        rc = GetExitCodeThread( WorkerThreadHandle, (LPDWORD) &code );
        if ( 0 == rc )
            dbgprint("StopAsyncWindowManager: Unable to retrieve thread exit code: %d",
                    GetLastError() );
        else if ( 0 != code )
            dbgprint("StopAsyncWindowManager: Async window thread exited abnormally!");

cleanup:

        CloseHandle( WorkerThreadHandle );

        WorkerThreadHandle = NULL;
    }

    return 0;
}

//
// Function: GetWorkerWindow
//
// Description:
//    This returns a handle to our hidden window that acts as an 
//    intermediary between the apps window and winsock. If the window
//    hasn't already been created, then create it.
//
HWND 
GetWorkerWindow(
    )
{
    HANDLE  ReadyEvent = NULL;
    int     rc;

    EnterCriticalSection( &gCriticalSection );
	if ( NULL == WorkerThreadHandle ) 
    {
        // Create an event which the worker thread will signal when its ready
        ReadyEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( NULL == ReadyEvent )
        {
            dbgprint("GetWorkerWindow: CreateEvent failed: %d", GetLastError() );
            goto cleanup;
        }

        // Create the asyn window message thread
        WorkerThreadHandle = CreateThread(
                NULL, 
                0, 
                AsyncMsgHandler, 
                (LPVOID)ReadyEvent, 
                0, 
                NULL
                ); 
        if ( NULL == WorkerThreadHandle )
        {
            dbgprint( "GetWorkerWindow: CreateThread failed: %d", GetLastError() );
            goto cleanup;
        }

        // Wait for the window to become initialized
        rc = WaitForSingleObject( ReadyEvent, INFINITE );
        if ( ( WAIT_FAILED == rc ) || ( WAIT_TIMEOUT == rc ) )
            dbgprint( "GetWorkerWindow: WaitForSingleObject failed: %d! (error = %d)", 
                    rc, GetLastError() );
    }

cleanup:

    if ( NULL != ReadyEvent )
    {
        // Close the ready event
        rc = CloseHandle( ReadyEvent );
        if ( 0 == rc )
        {
            dbgprint("GetWorkerWindow: CloseHandle failed: %d", GetLastError() );
        }
    }

    LeaveCriticalSection( &gCriticalSection );

	return AsyncWindow;
}

//
// Function: AsyncMsgHandler
//
// Description:
//    This is the message pump for our hidden window.
//
static DWORD WINAPI 
AsyncMsgHandler(
    LPVOID lpParameter
    )
{
	MSG      msg;
	DWORD    Ret;
    HANDLE   readyEvent;
    HRESULT  hr;
    WNDCLASS wndclass;


    // Event to signal when window is ready
    readyEvent = (HANDLE) lpParameter;

    hr = StringCchPrintf(
            AsyncProviderClassName,
            PROVIDER_CLASS_CHAR_LEN,
            PROVIDER_CLASS,
            GetCurrentProcessId()
            );
    if ( FAILED( hr ) )
    {
        dbgprint("AsyncMsgHandler: StringCchPrintf failed: %d", GetLastError());
        goto cleanup;
    }

    dbgprint("AsyncMsgHandler: Class name is '%s'", AsyncProviderClassName );

    memset( &wndclass, 0, sizeof( wndclass ) );
    wndclass.lpfnWndProc = (WNDPROC)AsyncWndProc;
    wndclass.hInstance = gDllInstance;
    wndclass.lpszClassName = AsyncProviderClassName;

    if ( 0 == RegisterClass( &wndclass ) )
    {
        dbgprint("AsyncMsgHandle: RegisterClass failed: %d", GetLastError());
        goto cleanup;
    }

    // Create a window.
    AsyncWindow = CreateWindow(
        AsyncProviderClassName,
        TEXT("Layered Hidden Window"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        gDllInstance,
        NULL
        );

    if ( NULL == AsyncWindow )
    {
        dbgprint("AsyncMessageHandler: CreateWindow failed: %d", GetLastError() );
        goto cleanup;
    }

    // Indicate the window is ready
    SetEvent( readyEvent );

    // Message pump
	while ( ( Ret = GetMessage( &msg, NULL, 0, 0 ) ) )
	{
		if ( -1 == Ret )
		{
            dbgprint("AsyncMessageHandler: GetMessage returned -1, exiting loop");
            break;
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

    // Clean up the window and window class which were created in this thread
    if ( NULL != AsyncWindow )
    {
        DestroyWindow( AsyncWindow );
        AsyncWindow = NULL;
    }

    UnregisterClass( AsyncProviderClassName, gDllInstance );

    ExitThread(0);

cleanup:

    SetEvent( readyEvent );

    ExitThread( (DWORD) -1);
}

//
// Function: AsyncWndProc
//
// Description:
//    This is the window proc for our hidden window. Once we receive a message
//    on our hidden window we must translate our lower provider's socket handle
//    to the app's socket handle and then complete the notification to the user.
//
static LRESULT CALLBACK 
AsyncWndProc(
	HWND    hWnd,
	UINT    uMsg,
	WPARAM  wParam,
	LPARAM  lParam
    )
{
	SOCK_INFO *si = NULL;
    int        retries,
               rc;

	if ( WM_SOCKET == uMsg )
	{
        retries = 0;

        //
        // Given the lower provider's socket handle (the wParam), do a reverse lookup
        // to find the socket context structure. It is possible we received a message
        // for a socket not in our list of sockets. This usually occurs after WSPAccept
        // completes and we immediately get a message for the accepted socket but the
        // LSP code to insert the new context object hasn't executed yet. If this is
        // the case wait on an event until a new context object is added (which 
        // signals the event). Note we'll wait only MAX_ASYNC_RETRIES times before
        // bailing.
        //
        while ( retries < MAX_ASYNC_RETRIES )
        {
            dbgprint("hWnd 0x%p, uMsg 0x%x, WPARAM 0x%p, LPARAM 0x%p (retries %d)",
                    hWnd, uMsg, wParam, lParam, retries);

            // Find the context for this lower provider socket handle
            si = GetCallerSocket( NULL, wParam );
            if ( NULL == si )
            {
                dbgprint("Unable to find socket context for 0x%p", wParam);

                // Wait on an event which is signaled when context is added
                rc = WaitForSingleObject( gAddContextEvent, ASYNC_TIMEOUT );
                if ( WAIT_FAILED == rc )
                {
                    dbgprint("AsyncWndProc: WaitForSingleObject failed: %d",
                    GetLastError());
                    break;
                }
                else // timeout or success
                {
                    // Reset the event if it was signaled
                    if ( WAIT_OBJECT_0 == rc )
                        ResetEvent( gAddContextEvent );

                    // If signaled, hopefully next lookup will succeed
                    retries++;
                    continue;
                }
            }

            gMainUpCallTable.lpWPUPostMessage(
                    si->hWnd, 
                    si->uMsg, 
                    si->LayeredSocket, 
                    lParam
                    );

            return 0;
        }
	}
    else if ( WM_DESTROY == uMsg )
    {
        // Post a quit message to exit our async message pump thread
        PostQuitMessage( 0 );
        return 0;
    }

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
