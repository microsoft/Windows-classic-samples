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

    PushSubscription.cpp

Abstract:

    This module implements a sample demonstrating how to use the push mode 
    Event Subscription.

Environment:

    Windows vista or above.

--*/

#include <windows.h>
#include <winevt.h>
#include <wchar.h>

#define BUFFER_SIZE 512

//
// Memory Allocation Macros.
//

#define G_ALLOC(Size)      HeapAlloc(GetProcessHeap(), 0, Size)
#define G_REALLOC(PreviousAddress, Size)  HeapReAlloc(GetProcessHeap(), 0, PreviousAddress, Size)
#define G_FREE(Address)       if (Address != NULL) HeapFree(GetProcessHeap(), 0, Address)

//
// User defined context to pass to the callback.
//

typedef struct _RENDER_CONTEXT {
    DWORD PropertiesCount;
    EVT_HANDLE RenderContext;
    DWORD EventCount;
} RENDER_CONTEXT, *PRENDER_CONTEXT;

//
// Event properties to render.
//

typedef enum _SYSTEM_PROPERTY_ID
{
    SystemProviderName = 0,
    SystemEventID,
    SystemTimeCreated
} SYSTEM_PROPERTY_ID;

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
    wprintf(L"%s [ChannelName XPathQueryString]\n\n",
            ExeName);
    wprintf(L"Example:\n %s Application *[System[(Level=2)]]\n",
            ExeName);
    wprintf(L"\tAbove example shows to subscribe to all the error events in Application channel.\n");
}

DWORD
RenderMessage(
    __in_opt EVT_HANDLE PublisherMetadata,
    __in EVT_HANDLE Event,
    __in DWORD Flags,
    __out PWSTR& Buffer)

/*++

Routine Description:

    This function generates message string associated with the flag
    for given provider.

Parameters:

    PublisherMetadata - Supplies provider handle obtained through EvtOpenPublisherMetadata.

    Event - Supplies event handle obtained through subscription callback.

    Flags - Supplies flag associated with provider event property.

    Buffer - Receives Output buffer that receives the resulting message string.

Return Value:

    Win32 Errorcode.

--*/

{
    DWORD Error = ERROR_SUCCESS;
    DWORD BufferSize = BUFFER_SIZE;
    PWSTR PreviousBuffer;

    Buffer = (PWSTR)G_ALLOC(BufferSize*sizeof(WCHAR));

    if (Buffer == NULL)    {
        return ERROR_OUTOFMEMORY;
    }

    if (!EvtFormatMessage(PublisherMetadata,
                          Event,
                          0,
                          0,
                          NULL,
                          Flags,
                          BufferSize,
                          (LPWSTR)Buffer,
                          &BufferSize)) {
        Error = GetLastError();
        if (Error == ERROR_INSUFFICIENT_BUFFER) {
            PreviousBuffer = Buffer;
            Buffer = (PWSTR)G_REALLOC(PreviousBuffer,
                                      BufferSize * sizeof(WCHAR));
            if (Buffer == NULL) {
                G_FREE(PreviousBuffer);
                return ERROR_OUTOFMEMORY;
            }
            Error = ERROR_SUCCESS;
            if (!EvtFormatMessage(PublisherMetadata,
                                  Event,
                                  0,
                                  0,
                                  NULL,
                                  Flags,
                                  BufferSize,
                                  (LPWSTR)Buffer,
                                  &BufferSize)) {
                Error = GetLastError();
                G_FREE(Buffer);
                Buffer = NULL;
            }
        } else {
            G_FREE(Buffer);
            Buffer = NULL;
        }
    }
    return Error;
}

DWORD WINAPI
SubscriptionCallBack( 
    __in EVT_SUBSCRIBE_NOTIFY_ACTION Action, 
    __in PVOID Context, 
    __in EVT_HANDLE Event
    )

/*++

Routine Description:

    This function provides Subscription callback used by the push
    subscription model.

Parameters:

    Action - Supplies  value that specifies whether or not event was retrieved.

    Context - Supplies User context.

    Event - Supplies handle to the event for which the callback was invoked.

Return Value:

    Win32 Errorcode.

--*/

{
    PBYTE EventPropertyBuffer;
    PBYTE PreviousEventPropertyBuffer;
    DWORD BufferSize = BUFFER_SIZE;
    DWORD BufferUsed = 0;
    DWORD Error = ERROR_SUCCESS;
    DWORD PropertyCount = 0;
    PRENDER_CONTEXT Callbackcontext;
    PEVT_VARIANT EventPropertyValues;
    SYSTEMTIME EventTimeStampSystemTime;
    FILETIME EventTimeStampFileTime;
    ULONGLONG Temp;
    PWSTR PropertyDescription;
    EVT_HANDLE Provider = NULL;
    
    //
    // When the callback cannot retrieve event, the event
    // handle returns win32 errorcode specifying why retrieving
    // event failed.
    //

    if (Action == EvtSubscribeActionError) {
        Error = reinterpret_cast<DWORD>(Event);
        wprintf(L"Could not get Event!. Error = 0x%x", Error);
        return Error;
    }

    Callbackcontext = (PRENDER_CONTEXT)Context;

    EventPropertyBuffer = (PBYTE)G_ALLOC(BufferSize);

    if (EventPropertyBuffer == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    //
    // Get the Event properties.
    //

    if (!EvtRender(Callbackcontext->RenderContext,
                   Event,
                   EvtRenderEventValues,
                   BufferSize,
                   EventPropertyBuffer,
                   &BufferUsed,
                   &PropertyCount)) {
        Error = GetLastError();
        if (Error == ERROR_INSUFFICIENT_BUFFER) {

            //
            // Allocate the buffer size needed to for the properties.
            //

            BufferSize = BufferUsed;
            PreviousEventPropertyBuffer = EventPropertyBuffer;
            EventPropertyBuffer = (PBYTE)G_REALLOC(PreviousEventPropertyBuffer,
                                                   BufferSize);
            if (EventPropertyBuffer == NULL) {
                G_FREE(PreviousEventPropertyBuffer);
                return ERROR_OUTOFMEMORY;
            }
            Error = ERROR_SUCCESS;
            
            //
            // Get the Event properties.
            //

            if (!EvtRender(Callbackcontext->RenderContext,
                           Event,
                           EvtRenderEventValues,
                           BufferSize,
                           EventPropertyBuffer, 
                           &BufferUsed,
                           &PropertyCount)) {
                Error = GetLastError();
                wprintf(L"Could not Render Event Properties!. Error = 0x%x",
                        Error);
                G_FREE(EventPropertyBuffer);
                return Error;
            }
        } else {
            G_FREE(EventPropertyBuffer);
        }
    }

    EventPropertyValues = (PEVT_VARIANT)EventPropertyBuffer;

    if (PropertyCount < Callbackcontext->PropertiesCount) {
        wprintf(L"Could not Render Events Properties!. Error = 0x%x",
                ERROR_INVALID_DATA);
        G_FREE(EventPropertyBuffer);
        return ERROR_INVALID_DATA;
    }

    wprintf(L"Event[%d]:\n",
            Callbackcontext->EventCount++);
    wprintf(L"  Provider: %s\n",
            EventPropertyValues[SystemProviderName].StringVal);
    wprintf(L"  Event ID: %u\n",
            EventPropertyValues[SystemEventID].UInt16Val);

    Temp = EventPropertyValues[SystemTimeCreated].FileTimeVal;
    EventTimeStampFileTime.dwHighDateTime = (DWORD)((Temp >> 32) & 0xFFFFFFFF);
    EventTimeStampFileTime.dwLowDateTime = (DWORD)(Temp & 0xFFFFFFFF);

    if (FileTimeToSystemTime(&EventTimeStampFileTime,&EventTimeStampSystemTime)) {
        wprintf(L"  Date: %02d/%02d/%d  %02d:%02d\n",
                EventTimeStampSystemTime.wMonth,
                EventTimeStampSystemTime.wDay,
                EventTimeStampSystemTime.wYear,
                EventTimeStampSystemTime.wHour,
                EventTimeStampSystemTime.wMinute);
    } else {
        wprintf(L"  Date: N/A\n");
    }

    //
    // If we cannot open provider handle, we use default provider by passing NULL 
    // to RenderMessage. Also, it is recommended to use some sort of cache for
    // provider handle, for the sake of simplicity this example does not add cache.
    //

    Provider = EvtOpenPublisherMetadata(NULL,
                                        EventPropertyValues[SystemProviderName].StringVal,
                                        NULL,
                                        0,
                                        0);

    G_FREE(EventPropertyBuffer);

    Error = RenderMessage(Provider,
                          Event,
                          EvtFormatMessageLevel,
                          PropertyDescription);

    if (Error == ERROR_SUCCESS) {
        wprintf(L"  Level: %s\n", 
                PropertyDescription);
        G_FREE(PropertyDescription);
    } else {
        wprintf(L"  Level: N/A\n");
    }

    Error = RenderMessage(Provider,
                          Event,
                          EvtFormatMessageTask,
                          PropertyDescription);
    
    if (Error == ERROR_SUCCESS) {
        wprintf(L"  Task: %s\n",
                PropertyDescription);
        G_FREE(PropertyDescription);
    } else {
        wprintf(L"  Task: N/A\n");
    }

    Error = RenderMessage(Provider,
                          Event,
                          EvtFormatMessageEvent,
                          PropertyDescription);

    if (Error == ERROR_SUCCESS) {
        wprintf(L"  Event Description: %s\n\n",
                PropertyDescription);
        G_FREE(PropertyDescription);
    } else {
        wprintf(L"  Event Description: N/A\n\n");
    }

    if (Provider != NULL) {
        EvtClose(Provider);
    }
    return Error;
}

DWORD __cdecl
wmain(
    __in int argc,
    __in_ecount(argc) PWSTR *argv
    )
    
/*++

Routine Description:

    The function is a main routine of the push
    subscription example.

Parameters:

    argc - Supplies total argument in commandline.

    argv - Supplies argument array for the commandline.

Return Value:

    Win32 Errorcode.

--*/

{
    EVT_HANDLE SubscrptionHandle;
    RENDER_CONTEXT UserContext;
    WCHAR* Channel = L"Application";
    WCHAR* Query = L"*";
    WCHAR CommandlineInput;
    DWORD Error = ERROR_SUCCESS;
    
    PCWSTR RenderContextProperties[] = {
        L"Event/System/Provider/@Name",
        L"Event/System/EventID",
        L"Event/System/TimeCreated/@SystemTime"
    };

    //
    // If two arguments are passed to the executable, we assume
    // the first one is channel and second one is query else we
    // print help.
    //

    if (argc == 3) {
        Channel = argv[1];
        Query = argv[2];
    } else if (argc != 1) {
        DisplayHelp(argv[0]);
        return ERROR_INVALID_DATA;
    }

    //
    // Create User context.
    //

    UserContext.PropertiesCount = sizeof(RenderContextProperties)/sizeof(PWSTR);
    UserContext.EventCount = 0;

    UserContext.RenderContext = EvtCreateRenderContext(UserContext.PropertiesCount,
                                                       RenderContextProperties,
                                                       EvtRenderContextValues);

    if (UserContext.RenderContext == NULL) {
        Error = GetLastError();
        wprintf(L"Could not create RenderContext. Error = 0x%x",
                Error);
        return Error;
    }

    //
    // Register the subscription.
    //

    SubscrptionHandle = EvtSubscribe(NULL, 
                                     NULL,
                                     Channel,
                                     Query,
                                     NULL,
                                     &UserContext,
                                     (EVT_SUBSCRIBE_CALLBACK) SubscriptionCallBack, 
                                     EvtSubscribeToFutureEvents);

    if (SubscrptionHandle == NULL) {
        Error = GetLastError();
        EvtClose(UserContext.RenderContext);
        wprintf(L"Could not Subscribe to Events!. Error = 0x%x",
                Error);
        return Error;
    }

    wprintf(L"NOTE: Hit 'Q' or 'q' to stop the event subscription\n");
    wprintf(L"Subscribing to %s channel with %s query\n\n",
            Channel,
            Query);

    do {
        CommandlineInput = _getwch();
        CommandlineInput = towupper(CommandlineInput);
    } while(CommandlineInput != 'Q');

    //
    // Close the subscriber handle first followed by RenderContext. 
    // This gurantees that rendercontext does not go away while subscription
    // callback is using it.
    //

    EvtClose(SubscrptionHandle);
    EvtClose(UserContext.RenderContext);

    return Error;
}