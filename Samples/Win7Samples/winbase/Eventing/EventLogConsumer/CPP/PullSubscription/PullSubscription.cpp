//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

/*++

Module Name:

    PullSubscription.cpp

Abstract:

    This module implements a sample demonstrating how to use pull mode 
    Event Subscription.

Environment:

    Windows vista or above.

--*/

#include <windows.h>
#include <winevt.h>
#include <stdio.h>
#include <wchar.h>

//
// N.B.
//    Channel - The subsciption channel name.
//    Query   - An XPath event query string.
//    BookMarkXml - A BookMark string which will be used to subscribe after a bookmark.
//    SignalEvent - A NT event handle used to syncronize the work thread and main thread.
//    Stop - Indicating if the subscription needs to pause.
//    ErrorCode - The error code associated with the subscription operation.
//

typedef struct _SUBSCRIBE_PARAMETER {
    PCWSTR Channel;
    PCWSTR Query;
    PCWSTR BookMarkXml;
    HANDLE SignalEvent;
    BOOL   Stop;
    DWORD  ErrorCode;
} SUBSCRIBE_PARAMETER, *PSUBSCRIBE_PARAMETER;

VOID
DisplayHelp (
    __in PCWSTR ExeName
    )

/*++

Routine Description:

    This function displays the usage of the sample tool.

Parameters:

    ExeName - Supplies the sample tool executable name.

Return Value:

    None.

--*/
{
    wprintf(L"%s <ChannelName> [XPathQueryString]\n", ExeName);
    wprintf(L"Example: %s Application *[System[(Level=2)]]\n", ExeName);
    wprintf(L"\tAbove example shows to subscribe to all the error events in Application channel.\n");
}

DWORD
RenderEvent (
    __in EVT_HANDLE Fragment,
    __in EVT_RENDER_FLAGS Flags,
    __out PCWSTR* RenderedXML
    )

/*++

Routine Description:

    This function renders an event to text or renders a bookmark 
    handle to XML text.

Parameters:

    Fragment - Supplies the event handle or the bookmark handle.

    Flags - Supplies the flag for rendering function.

    RenderedXML - Retrieves the rendered XML text string.

Return Value:

    Returns ERROR_SUCCESS means operation succeeds. Other Win32 error
    code indicating the detail error.

--*/

{
    const DWORD DefaultSize = 2048;

    DWORD SizeNeeded;
    PVOID Buffer;
    PVOID NewBuffer;

    //
    // Allocate the memory based on the default size first.
    //

    Buffer = HeapAlloc(GetProcessHeap(), 0, DefaultSize);
    if (Buffer == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Return to the caller if the default size is enough to hold 
    // the final result.
    //

    if (EvtRender(NULL,
                  Fragment,
                  Flags,
                  DefaultSize,
                  Buffer,
                  &SizeNeeded,
                  NULL)) {
        *RenderedXML = (PCWSTR) Buffer;
        return ERROR_SUCCESS;
    }

    //
    // If EvtRender fails with unknown error other than 
    // ERROR_INSUFFICIENT_BUFFER, return the error directly.
    //

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        HeapFree(GetProcessHeap(), 0, Buffer);
        return GetLastError();
    }

    //
    // Reallocate the memory according to the real needed size and 
    // call EvtRender again.
    //

    NewBuffer = HeapReAlloc(GetProcessHeap(), 0, Buffer, SizeNeeded);
    if (NewBuffer == NULL) {
        HeapFree(GetProcessHeap(), 0, Buffer);
        return ERROR_OUTOFMEMORY;
    }
    Buffer = NewBuffer;

    if (!EvtRender(NULL,
                   Fragment,
                   Flags,
                   SizeNeeded,
                   Buffer,
                   &SizeNeeded,
                   NULL)) {
        HeapFree(GetProcessHeap(), 0, Buffer);
        return GetLastError();
    }

    *RenderedXML = (PCWSTR) Buffer;
    return ERROR_SUCCESS;
}

VOID
PrintEventXML (
    __in EVT_HANDLE Event
    )

/*++

Routine Description:

    This function prints the event XML string to the console.

Parameters:

    Event - Supplies the event handle.

Return Value:

    None.

--*/

{
    PCWSTR RenderedXML;
    DWORD  Error;

    Error = RenderEvent(Event, EvtRenderEventXml, &RenderedXML);
    if (Error == ERROR_SUCCESS) {
        wprintf(L"\n%s\n", RenderedXML);
        HeapFree(GetProcessHeap(), 0, (PVOID) RenderedXML);
    } else {
        wprintf(L"\nRenderEventFail with error code=%u\n", Error);
    }
}

DWORD
SaveBookmarkXML (
    __in  EVT_HANDLE BookMark,
    __out PCWSTR* BookMarkXML
    )

/*++

Routine Description:

    This function saves the bookmark text from the bookmark handle.

Parameters:

    BookMark - Supplies the bookmark handle.

    BookMarkXML - Retrieves the bookmark XML text.

Return Value:

    Returns ERROR_SUCCESS means operation succeeds. Other Win32 error
    code indicating the detail error.

--*/

{
    return RenderEvent(BookMark, EvtRenderBookmark, BookMarkXML);
}


VOID CALLBACK
PullSubscription (
    __inout     PTP_CALLBACK_INSTANCE Instance,
    __inout_opt PVOID Context,
    __inout     PTP_WORK Work
    )

/*++

Routine Description:

    This function implements the main logic of a pull subscription
    of a channel in event log.

Parameters:

    Instance  - Supplies a TP_CALLBACK_INSTANCE structure that defines
        the callback instance.

    Context - Supplies the application-defined data.

    Work - Supplies a TP_WORK structure that defines the work object that 
        generated the callback.

Return Value:

    None.

--*/

{
    PSUBSCRIBE_PARAMETER Parameter;

    EVT_HANDLE Subscription = NULL;
    EVT_HANDLE BookMark = NULL;

    const DWORD BatchSize = 16;
    EVT_HANDLE Events[BatchSize];
    DWORD ReturnedNumber;

    UNREFERENCED_PARAMETER(Instance);
    UNREFERENCED_PARAMETER(Work);

    Parameter = (PSUBSCRIBE_PARAMETER) Context;
    Parameter->ErrorCode = ERROR_SUCCESS;

    //
    // Create the bookmark handle.
    //

    BookMark = EvtCreateBookmark(Parameter->BookMarkXml);
    if (BookMark == NULL) {
        Parameter->ErrorCode = GetLastError();
        goto Exit;
    }

    //
    // Create a pull subscription.
    //
    // N.B. When the callback parameter is NULL, it is a pull subscription.
    //      When the bookmark passed in is the one saved last time, the
    //      subscription will be able to continue from where it stops last
    //      time.
    //

    Subscription = EvtSubscribe(NULL,
                                Parameter->SignalEvent,
                                Parameter->Channel,
                                Parameter->Query,
                                BookMark,
                                NULL,
                                NULL,
                                EvtSubscribeStartAfterBookmark);

    if (Subscription == NULL) {
        Parameter->ErrorCode = GetLastError();
        goto Exit;
    }

    //
    // Keep the subscription working if the Stop flag is not set.
    //

    while (!Parameter->Stop) {

        //
        // As long as EvtNext can return events, keep consuming them.
        //

        while (EvtNext(Subscription,
                       BatchSize,
                       Events,
                       INFINITE,
                       0,
                       &ReturnedNumber)) {
               for (DWORD i = 0; i < ReturnedNumber; i++) {
                    PrintEventXML(Events[i]);
                    if (BookMark != NULL) {
                        EvtUpdateBookmark(BookMark, Events[i]);
                    }
                    EvtClose(Events[i]);
               }

               //
               // User needs to stop the subscription, then exit.
               //

               if (Parameter->Stop) {
                   goto Exit;
               }
        }

        //
        // Subscription meets some un expected error, walk away.
        //

        if (GetLastError() != ERROR_NO_MORE_ITEMS) {
            Parameter->ErrorCode = GetLastError();
            break;
        }

        //
        // If no more events coming, wait on the SingalEvent to either get the
        // next event delivery notification or be singnaled by the user to stop
        // the subscription.
        //

        if (WaitForSingleObject(Parameter->SignalEvent, INFINITE) != WAIT_OBJECT_0) {
            Parameter->ErrorCode = GetLastError();
            break;
        }
    }

Exit:
    if (Subscription != NULL) {
        EvtClose(Subscription);
    }
    if (BookMark != NULL) {

        //
        // Save the bookmark XML into the parameter so that next time
        // a subscription can continue from where it stops. 
        //
        // N.B. Free the existing bookmark string first before we update
        // bookmark to the new one to prevent memory leaks.
        //

        if (Parameter->BookMarkXml != NULL) {
            HeapFree(GetProcessHeap(), 0, (PVOID) Parameter->BookMarkXml);
            Parameter->BookMarkXml = NULL;
        }
        DWORD Error = SaveBookmarkXML(BookMark, &Parameter->BookMarkXml);

        //
        // Below if is used to prevent potential overwrite the existing error
        // code from previous operations.
        //

        if (Parameter->ErrorCode == ERROR_SUCCESS) {
            Parameter->ErrorCode = Error;
        }
        EvtClose(BookMark);
    }
}

DWORD __cdecl
wmain (
    __in int argc,
    __in_ecount(argc) PWSTR argv[]
    )

/*++

Routine Description:

    The main entry function of the sample.

Parameters:

    argc - Supplies the number of the command arguments.

    argv - Supplies the real arguments strings.

Return Value:

    The function returns ERROR_SUCCESS means success. It returns other
    windows defined error code indicating the detail error.

--*/

{
    SUBSCRIBE_PARAMETER SubcriptionParameter;
    PTP_WORK Work;
    WCHAR ControlCharacter;
    BOOL Done = FALSE;

    if (argc < 2 ||
       (wcscmp(argv[1], L"-h") == 0) ||
       (wcscmp(argv[1], L"-?") == 0) ||
       (wcscmp(argv[1], L"/h") == 0) ||
       (wcscmp(argv[1], L"/?") == 0)) {

        DisplayHelp(argv[0]);
        return ERROR_SUCCESS;
    }

    //
    // Set the parameters for the subscription.
    //

    ZeroMemory(&SubcriptionParameter, sizeof(SubcriptionParameter));
    SubcriptionParameter.Channel = argv[1];
    if (argc >= 3) {
        SubcriptionParameter.Query = argv[2];
    }

    //
    // Create a thread pool work item to do the subsciption work,
    // while the main thread can control the progress of the work.
    //

    Work = CreateThreadpoolWork(PullSubscription,
                                &SubcriptionParameter,
                                NULL);
    if (Work == NULL) {
        wprintf(L"error=%u\n", GetLastError());
        return GetLastError();
    }

    //
    // Create a NT signal event which will be used in subscription.
    // 

    SubcriptionParameter.SignalEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (SubcriptionParameter.SignalEvent == NULL) {
        CloseThreadpoolWork(Work);
        return GetLastError();
    }

    SubmitThreadpoolWork(Work);

    while (!Done) {

        //
        // Wait for user's input.
        //

        ControlCharacter = _getwch();

        //
        // If user puts s, it means he wants the subscribption to stop.
        //

        if (ControlCharacter == L's' || ControlCharacter == 'S') {
            SubcriptionParameter.Stop = TRUE;

            //
            // Wait for the work to finish himself and then we can 
            // collect the error code.
            //

            SetEvent(SubcriptionParameter.SignalEvent);
            WaitForThreadpoolWorkCallbacks(Work, TRUE);

            wprintf(L"Subscription stops with last result code: %u\n", SubcriptionParameter.ErrorCode);
            wprintf(L"Press any key to resume the subsciption...\n");
            _getwch();

            ResetEvent(SubcriptionParameter.SignalEvent);
            SubcriptionParameter.Stop = FALSE;

            SubmitThreadpoolWork(Work);
            wprintf(L"Subscription has been resumned\n");
            continue;
        }

        //
        // If user puts q, it menas he wants to end the whole process.
        //

        if (ControlCharacter == L'q' || ControlCharacter == L'Q') {
            Done = TRUE;
            SubcriptionParameter.Stop = TRUE;

            SetEvent(SubcriptionParameter.SignalEvent);
            WaitForThreadpoolWorkCallbacks(Work, TRUE);

            wprintf(L"Subsciption finishes with exit code: %u\n",
                    SubcriptionParameter.ErrorCode);
        }
    }

    if (SubcriptionParameter.BookMarkXml != NULL) {
        HeapFree(GetProcessHeap(), 0, (PVOID) SubcriptionParameter.BookMarkXml);
    }
    CloseHandle(SubcriptionParameter.SignalEvent);
    CloseThreadpoolWork(Work);
    return SubcriptionParameter.ErrorCode;
}
