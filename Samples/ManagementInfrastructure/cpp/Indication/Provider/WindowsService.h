//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <stdlib.h>
#include <windows.h>
#include <MI.h>

// A thread used to polling all services to detect
// service stopped and started event (i.e, indication)
extern HANDLE g_hPollingThread;

// A critical section used to protect the modification of g_hPollingThread
extern CRITICAL_SECTION g_csPollingThread;

// An event used to shutdown the polling thread
extern HANDLE g_hStopPollingEvent;

// A count to remember how many indication classes are relying on
// polling thread now. Since we only have 2 indication class, thus
// the number could be 0, 1, 2.
//
//   Once the number became from 1 to 0, current thread need to shutdown the
//   polling thread by signal g_hStopPollingEvent;
//
//   Once the number became from 0 to 1, current thread need to
//   creat an polling thread.
extern volatile LONG g_nActiveIndicationClass;

// A polling thread context structure
typedef struct __PollingThreadArgument
{
    // Each indication class has its own context,
    // so we difine 2 context here to post
    // indications back to client.
    // PollingThread will post back indications
    // as long as corresponding context is NOT null.
    MI_Context* contextForStarted;
    MI_Context* contextForStopped;

}PollingThreadArgument;

// Polling thread procedure
DWORD WINAPI ThreadProc(__in LPVOID lpParameter);

// A linked list structure that store the snapshot of windows services status
typedef struct __WindowsService
{
    LPWSTR pName;
    DWORD dwState;
    struct __WindowsService * pNextService;
}WindowsService;

// Helper function used to enable ServiceStarted indication
MI_Result EnableServiceStartedIndication(__in MI_Context *context,
                                         _In_opt_z_ const MI_Char *lpwszNamespace);

// Helper function used to disable ServiceStarted indication
MI_Result DisableServiceStartedIndication(__in MI_Context *context);

// Helper function used to enable ServiceStopped indication
MI_Result EnableServiceStoppedIndication(__in MI_Context *context,
                                         _In_opt_z_ const MI_Char *lpwszNamespace);

// Helper function used to disable ServiceStopped indication
MI_Result DisableServiceStoppedIndication(__in MI_Context *context);

// Initialize global variables, which will be invoked once this provider was
// being loaded, see module.c : Load
MI_Result Initialize();

// Finalize global variables, which will be invoked once this provider was
// being unloaded, see module.c : Unload
MI_Result Finalize();

// Helper functions used to search windows service entry from snapshot
DWORD FindAndAddIfNotFound(
    __in_z LPCWSTR pServiceName,
    __out BOOL * pFound,
    __deref_out_opt WindowsService** ppService);

// The polling thread used to poll service status and generate
// both MSFT_WindowsServiceStarted and MSFT_WindowsServiceStopped indication
// on demand.
DWORD WINAPI PollingThreadProc(__in  LPVOID lpParameter);