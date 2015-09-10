//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <Windows.h>
#include <malloc.h>
#include <MI.h>
#include <assert.h>
#include <string.h>
#include "WindowsService.h"
#include "MSFT_WindowsServiceStarted.h"
#include "MSFT_WindowsServiceStopped.h"

// Define the polling interval value (in milliseconds)
#define POLLING_INTERVAL 5000

// A thread used to polling all services to detect
// service stopped and started event (i.e, indication)
HANDLE g_hPollingThread = NULL;

// A critical section used to protect the modification of g_hPollingThread
CRITICAL_SECTION g_csPollingThread;

// An event used to shutdown the polling thread
HANDLE g_hStopPollingEvent = NULL;

// Remember the Namespace into which this provider is loaded
volatile LPWSTR g_lpwszNamespace = NULL;

// Query template used to build query
static LPCWSTR g_pQueryTemplate = L"select * from MSFT_WindowsService where Name='";
SIZE_T g_nQueryTemplateLength = 0;

//
// A count to remember how many indication classes are relying on
// polling thread now. Since we only have 2 indication class, thus
// the number could be 0, 1, 2.
//
//   Once the number became from 1 to 0, current thread need to shutdown the
//   polling thread by signal g_hStopPollingEvent;
//
//   Once the number became from 0 to 1, current thread need to
//   creat an polling thread.
//
volatile LONG g_nActiveIndicationClass = 0;

//
// Polling thread rely on a snapshot of services to detect any status change of
// service, g_ServiceHeader is a linked list which hold a snapshot of all
// windows services of the last polling. With each polling, the Polling thread
// will update the status of corresponding service and trigger the corresponding
// indication (event).
//
WindowsService g_ServiceHeader;

//
// Asssert Helper function
//
#define MY_DEBUG
void MyAssert(BOOL exp)
{
    if (exp == FALSE)
    {
        #ifdef MY_DEBUG
        *(int *)(0) = 0;
        #else
        assert(exp);
        #endif
    }
}

//
// Helper function of allocating memory from process heap
// 
// Argument:
//      dwBytes     number of bytes to allocate.
//  
// Return value:
//      allocated memory address
//
LPVOID AllocateMemory(SIZE_T dwBytes)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytes);
}

//
// Helper function of freeing memory
// 
// Argument:
//      lpMem       memory address to free.
//
void FreeMemory(LPVOID lpMem)
{
    HeapFree(GetProcessHeap(), 0, lpMem);
}

//
// Initialize global variables, which will be invoked once this provider was
// being loaded, see module.c : Load
//
// Return value:
//      MI_RESULT_OK means success, otherwise failed
//
MI_Result Initialize()
{
    memset(&g_ServiceHeader, 0, sizeof(WindowsService));
    InitializeCriticalSection(&g_csPollingThread);
    g_hStopPollingEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_hStopPollingEvent == NULL)
    {
        return MI_RESULT_FAILED;
    }
    g_nQueryTemplateLength = wcslen(g_pQueryTemplate);
    return MI_RESULT_OK;
}

//
// Finalize global variables, which will be invoked once this provider was
// being unloaded, see module.c : Unload
//
// Return value:
//      MI_RESULT_OK means success, otherwise failed
//
MI_Result Finalize()
{
    MyAssert(g_hStopPollingEvent != NULL);
    {
        WindowsService *pService = g_ServiceHeader.pNextService;
        while (pService != NULL)
        {
            FreeMemory(pService->pName);
            {
                LPVOID pTemp = pService;
                pService = pService->pNextService;
                FreeMemory(pTemp);
            }
        }
        memset(&g_ServiceHeader, 0, sizeof(WindowsService));
    }
    if (g_lpwszNamespace != NULL)
    {
        FreeMemory(g_lpwszNamespace);
        g_lpwszNamespace = NULL;
    }
    if(g_hStopPollingEvent)
    {
        CloseHandle(g_hStopPollingEvent);
        g_hStopPollingEvent = NULL;
    }
    DeleteCriticalSection(&g_csPollingThread);
    return MI_RESULT_OK;
}



//
// Global PollingThreadArgument. MSFT_WindowsServiceStarted and
// MSFT_WindowsServiceStopped rely on it to passing indication context to
// polling thread; Polling thread uses the argument to post the indication
// event to client through the context object.
//
PollingThreadArgument g_Arg;

//
// helper function used to start polling thread.
// it will create the polling thread if and only if
// the polling thread is not created yet, which can be
// tracked by the value of g_nActiveIndicationClass
//
void StartPollingThread()
{
    EnterCriticalSection(&g_csPollingThread);
    {
        g_nActiveIndicationClass ++;
        if (g_nActiveIndicationClass == 1)
        {
            // This is the first indication class, need to 
            // create the polling thread to start the polling
            MyAssert(g_hPollingThread == NULL);
            g_hPollingThread = CreateThread(NULL, 0, PollingThreadProc, &g_Arg, 0, 0);
            // SetThreadPriority(g_hPollingThread, THREAD_PRIORITY_LOWEST);
        }
    }
    LeaveCriticalSection(&g_csPollingThread);
}

//
// helper function used to shutdown polling thread.
// it will shutdown the polling thread if and only if
// there is no indication class rely on polling thread to generate indications,
// which can be tracked by the value of g_nActiveIndicationClass
//
void StopPollingThread()
{
    EnterCriticalSection(&g_csPollingThread);
    {
        g_nActiveIndicationClass --;
        if (g_nActiveIndicationClass == 0)
        {
            // there is the no active indication class,
            // need to shutdown the polling thread
            if (g_hPollingThread != NULL)
            {
                // Notify the polling thread to shutdown
                SetEvent(g_hStopPollingEvent);
                {
                    HANDLE hPollingThread = g_hPollingThread;
                    g_hPollingThread = NULL;
                    // Wait until the polling thread to shutdown
                    WaitForSingleObject(hPollingThread, INFINITE);
                    CloseHandle(hPollingThread);
                }
            }
        }
    }
    LeaveCriticalSection(&g_csPollingThread);
}

//
// Helper function used to set namespace into which the provider is loaded
// 
// Argument:
//      lpwszNamespace     The namespace that loaded provider.
//  
// Return value:
//      MI_RESULT_OK means set namespace successfully, otherwise failed.
//
MI_Result SetNamespace(_In_opt_z_ const MI_Char *lpwszNamespace)
{
    if (InterlockedCompareExchangePointer((PVOID*)(&g_lpwszNamespace), NULL, NULL) == NULL)
    {
        if (lpwszNamespace)
        {
            SIZE_T nNamespaceLength = wcslen(lpwszNamespace) + 1;
            LPWSTR lpwszNamespaceTemp = (LPWSTR)AllocateMemory(nNamespaceLength * sizeof(wchar_t));
            if (lpwszNamespaceTemp == NULL)
            {
                return MI_RESULT_SERVER_LIMITS_EXCEEDED;
            }
            wcscpy_s(lpwszNamespaceTemp, nNamespaceLength, lpwszNamespace);
            // set g_lpwszNamespace threadsafely
            if (InterlockedCompareExchangePointer(
                (PVOID*)(&g_lpwszNamespace),
                lpwszNamespaceTemp,
                NULL) != NULL)
            {
                FreeMemory(lpwszNamespaceTemp);
            }
        }
    }
    return MI_RESULT_OK;
}

//
// Helper function used to enable ServiceStarted indication
// 
// Argument:
//      context     The context used to post indication to client.
//  
// Return value:
//      Enable result
//
MI_Result EnableServiceStartedIndication(__in MI_Context *context,
                                         _In_opt_z_ const MI_Char *lpwszNamespace)
{
    MI_Result r = SetNamespace(lpwszNamespace);
    if (r != MI_RESULT_OK)
    {
        return r;
    }
    g_Arg.contextForStarted = context;
    StartPollingThread();
    
    return MI_RESULT_OK;
}

//
// Helper function used to disable ServiceStarted indication
// 
// Argument:
//      context     The context used to post indication to client.
//  
// Return value:
//      Disable result
//
MI_Result DisableServiceStartedIndication(__in MI_Context *context)
{
    MI_UNREFERENCED_PARAMETER(context);
    MyAssert(g_Arg.contextForStarted == context);
    g_Arg.contextForStarted = NULL;
    StopPollingThread();
    return MI_RESULT_OK;
}

//
// Helper function used to enable ServiceStopped indication
// 
// Argument:
//      context     The context used to post indication to client.
//  
// Return value:
//      Enable result
//
MI_Result EnableServiceStoppedIndication(__in MI_Context *context,
                                         _In_opt_z_ const MI_Char *lpwszNamespace)
{
    MI_Result r = SetNamespace(lpwszNamespace);
    if (r != MI_RESULT_OK)
    {
        return r;
    }
    g_Arg.contextForStopped = context;
    StartPollingThread();
    return MI_RESULT_OK;
}

//
// Helper function used to disable ServiceStopped indication
//
// Argument:
//      context     The context used to post indication to client.
//  
// Return value:
//      Disable result
//
MI_Result DisableServiceStoppedIndication(__in MI_Context *context)
{
    MI_UNREFERENCED_PARAMETER(context);
    MyAssert(g_Arg.contextForStopped == context);
    g_Arg.contextForStopped = NULL;
    StopPollingThread();
    return MI_RESULT_OK;
}

//
// Helper functions used to search windows service entry from snapshot.
//
// Parameter:
//      pServiceName    name of the service to search, which is unique on windows machine.
//      pFound          output parameter, which indicates whether a service was
//                      found in snapshot.
//      ppService       optional output parameter, which is the found service object.
//
// Return value:
//      FALSE means not found, TRUE means found and return the notepad
//      through ppService parameter.
//
DWORD FindAndAddIfNotFound(
    __in_z LPCWSTR pServiceName,
    __out BOOL * pFound,
    __deref_out_opt WindowsService** ppService)
{
    WindowsService * pService;

    MyAssert(pServiceName != NULL);
    MyAssert(ppService != NULL);
    MyAssert(pFound != NULL);

    *ppService = NULL;
    *pFound = FALSE;

    // search the given service from snapshot (linked list)
    pService = g_ServiceHeader.pNextService;
    while (pService != NULL)
    {
        if (_wcsicmp(pService->pName, pServiceName) == 0)
        {
            // found the service object, return
            *ppService = pService;
            *pFound = TRUE;
            return ERROR_SUCCESS;
        }
        pService = pService->pNextService;
    }

    // not found, create a new service object
    pService = (WindowsService*)AllocateMemory(sizeof(WindowsService));
    if (pService == NULL)
    {
        // out of memory, return 
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    {
        size_t nNameLength = wcslen(pServiceName) + 1;
        pService->pName = (wchar_t*)AllocateMemory(nNameLength * sizeof(wchar_t));
        if (pService->pName == NULL)
        {
            FreeMemory(pService);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        wcscpy_s(pService->pName, nNameLength, pServiceName);
    }

    // add the new service object to the snapshot
    {
        WindowsService *pNext = g_ServiceHeader.pNextService;
        g_ServiceHeader.pNextService = pService;
        pService->pNextService = pNext;
    }

    // return the new service object
    *ppService = pService;
    return ERROR_SUCCESS;
}

//
// Enumeration of the Service Status, see WinSvc.h
//  #define SERVICE_STOPPED                        0x00000001
//  #define SERVICE_START_PENDING                  0x00000002
//  #define SERVICE_STOP_PENDING                   0x00000003
//  #define SERVICE_RUNNING                        0x00000004
//  #define SERVICE_CONTINUE_PENDING               0x00000005
//  #define SERVICE_PAUSE_PENDING                  0x00000006
//  #define SERVICE_PAUSED                         0x00000007

// Service Status values
static MI_Char *SERVICE_STATUS_VALUE[] = {
    MI_T("Unknown"),
	MI_T("Stopped"),
	MI_T("Starting"),
	MI_T("Stopping"),
	MI_T("Running"),
	MI_T("ContinuePending"),
	MI_T("PausePending"),
	MI_T("Paused")
};

//
// Helper function used to read current system time
//
void GetSystemDateTime(__inout MI_Datetime * pDateTime)
{
    MyAssert(pDateTime != NULL);
    {
        SYSTEMTIME systemTime;
        GetSystemTime(&systemTime);
        pDateTime->isTimestamp = MI_TRUE;
        pDateTime->u.timestamp.year = systemTime.wYear;
        pDateTime->u.timestamp.month = systemTime.wMonth;
        pDateTime->u.timestamp.day = systemTime.wDay;
        pDateTime->u.timestamp.hour = systemTime.wHour;
        pDateTime->u.timestamp.minute = systemTime.wMinute;
        pDateTime->u.timestamp.second = systemTime.wSecond;
        pDateTime->u.timestamp.microseconds = systemTime.wMilliseconds;
        pDateTime->u.timestamp.utc = 0;
    }
}

//
// Helper function used to query MSFT_WindowsService instance and post back
// generated indication.
//
void SendIndication(
    __in MI_Context *miContext,
    __in DWORD dwIndicationType, // 0 - started indication, 1 -stopped indication
    __in DWORD dwOriginalState,
    _In_z_ const wchar_t *namespaceName,
    _In_z_ const wchar_t *serviceName)
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_Boolean moreResults;
    const MI_Char *errorString = NULL;
    const MI_Instance *errorDetails = NULL;
    MI_Session miSession;
    LPWSTR lpQueryBuffer = NULL;
    SIZE_T nQueryBufferLength = 0;
    SIZE_T nServiceNameLength = 0;
    MyAssert(serviceName != NULL);
    MI_UNREFERENCED_PARAMETER(serviceName);

    miResult = MI_Context_GetLocalSession(miContext, &miSession);
    if (miResult != MI_RESULT_OK)
    {
        // do not send indication if cannot get locale session
        return;
    }
    nServiceNameLength = wcslen(serviceName);
    nQueryBufferLength = nServiceNameLength + g_nQueryTemplateLength + 2;
    lpQueryBuffer = (LPWSTR)AllocateMemory(nQueryBufferLength * sizeof(wchar_t));
    if (lpQueryBuffer == NULL)
    {
        return;
    }
    wcscpy_s(lpQueryBuffer,
        nQueryBufferLength,
        g_pQueryTemplate);
    wcscpy_s(lpQueryBuffer + g_nQueryTemplateLength,
        nQueryBufferLength - g_nQueryTemplateLength,
        serviceName);
    lpQueryBuffer[nQueryBufferLength - 2] = L'\'';
    lpQueryBuffer[nQueryBufferLength - 1] = L'\0';

    // Query the MSFT_WindowsService instance based on service name,
    // since both indication event contains the instance object
    // as embedded property. In order to send indication event back
    // to client, it is mandatory to set both sourceInstance and
    // previousInstance properties.
    MI_Session_QueryInstances(&miSession,
        0,
        NULL,
        namespaceName,
        MI_T("WQL"),
        lpQueryBuffer,
        NULL,
        &miOperation);
    do
    {
        MI_Instance *miInstance;
        MI_Result _miResult;

        _miResult = MI_Operation_GetInstance(&miOperation,
            &miInstance,
            &moreResults,
            &miResult,
            &errorString,
            &errorDetails);

        if (_miResult != MI_RESULT_OK)
        {
            /* This means cannot read the service instance, skip issuing indication. */
            break;
        }
        if (miInstance)
        {
            if (dwIndicationType == 0)
            {
                MSFT_WindowsServiceStarted started;
                _miResult = MSFT_WindowsServiceStarted_Construct(&started, miContext);
                if (_miResult != MI_RESULT_OK) break;
                do
                {
                    // set sourceInstance property of indication object
                    MI_Value v;
                    MI_Datetime systemTime = {0};
                    v.string = MI_T("Running");
                    _miResult = MI_Instance_SetElement(miInstance, MI_T("Status"), &v, MI_STRING, 0);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    _miResult = MSFT_WindowsServiceStarted_Set_SourceInstance(&started, miInstance);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    // set previousInstance property of indication object
                    v.string = SERVICE_STATUS_VALUE[dwOriginalState];
                    _miResult = MI_Instance_SetElement(miInstance, MI_T("Status"), &v, MI_STRING, 0);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    _miResult = MSFT_WindowsServiceStarted_Set_PreviousInstance(&started, miInstance);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    GetSystemDateTime(&systemTime);
                    _miResult = MSFT_WindowsServiceStarted_Set_IndicationTime(&started, systemTime);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    // post indication back to client
                    MSFT_WindowsServiceStarted_Post(&started, miContext, 0, L"");
                } while (FALSE);
                MSFT_WindowsServiceStarted_Destruct(&started);
            }
            else if (dwIndicationType == 1)
            {
                MSFT_WindowsServiceStopped stopped;
                _miResult = MSFT_WindowsServiceStopped_Construct(&stopped, miContext);
                if (_miResult != MI_RESULT_OK) break;
                do
                {
                    // set sourceInstance property of indication object
                    MI_Value v;
                    MI_Datetime systemTime = {0};
                    v.string = MI_T("Stopped");
                    _miResult = MI_Instance_SetElement(miInstance, MI_T("Status"), &v, MI_STRING, 0);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    _miResult = MSFT_WindowsServiceStopped_Set_SourceInstance(&stopped, miInstance);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    // set previousInstance property of indication object
                    v.string = SERVICE_STATUS_VALUE[dwOriginalState];
                    _miResult = MI_Instance_SetElement(miInstance, MI_T("Status"), &v, MI_STRING, 0);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    _miResult = MSFT_WindowsServiceStopped_Set_PreviousInstance(&stopped, miInstance);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    GetSystemDateTime(&systemTime);
                    _miResult = MSFT_WindowsServiceStopped_Set_IndicationTime(&stopped, systemTime);
                    if (_miResult != MI_RESULT_OK)
                    {
                        break;
                    }
                    // post indication back to client
                    MSFT_WindowsServiceStopped_Post(&stopped, miContext, 0, L"");
                }
                while(FALSE);
                MSFT_WindowsServiceStopped_Destruct(&stopped);
            }
        }
    } while (FALSE); // we only expect one result instance
    FreeMemory(lpQueryBuffer);
    MI_Operation_Close(&miOperation);
}

//
// Helper function used to generate indication instance based on service status
// and post indication back to client.
//
void GenerateIndication(
    __in PollingThreadArgument * pArg,
    __in DWORD dwOriginalState,
    __in ENUM_SERVICE_STATUS * pServiceStatus)
{
    MyAssert(pArg != NULL);
    MyAssert(pServiceStatus != NULL);

    // generate the event if necessary
    if (pArg->contextForStarted != NULL)
    {
        if ((dwOriginalState !=  SERVICE_RUNNING) && 
           (pServiceStatus->ServiceStatus.dwCurrentState == SERVICE_RUNNING))
        {
            // trigger service started indication
            // SourceInstance & PreviousInstance should read from service
            // provider directly.
            SendIndication(pArg->contextForStarted,
                0,
                dwOriginalState,
                g_lpwszNamespace,
                pServiceStatus->lpServiceName);
        }
    }
    if (pArg->contextForStopped != NULL)
    {
        if ((dwOriginalState !=  SERVICE_STOPPED) && 
           (pServiceStatus->ServiceStatus.dwCurrentState == SERVICE_STOPPED))
        {
            // trigger service stopped indication
            // SourceInstance & PreviousInstance should read from service
            // provider directly.
            SendIndication(pArg->contextForStopped,
                1,
                dwOriginalState,
                g_lpwszNamespace,
                pServiceStatus->lpServiceName);
        }
    }
}

//
// This function is invoked from polling thread, which poll all services status
// and generate MSFT_WindowsServiceStarted and MSFT_WindowsServiceStopped indication
// on demand.
// 
// Argument:
//      lpParameter     parameter passed to polling thread
// 
// Return value:
//      Thread execution result
//
DWORD WINAPI PollingThreadProc(__in  LPVOID lpParameter)
{
    DWORD errorToReturn = ERROR_SUCCESS;
    SC_HANDLE hSvcCtlMgr;
    BOOL bShutdown = FALSE;
    PollingThreadArgument * pArg = (PollingThreadArgument *)lpParameter;
    assert (pArg != NULL);

    hSvcCtlMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);
    if (hSvcCtlMgr == NULL)
    {
        // Cannot get access to SCManager object and fail.
        return GetLastError();
    }

    do
    {
        DWORD dwServiceIndex, dwBytesNeeded, dwServiceCount, dwResumeHandle = 0;
        ENUM_SERVICE_STATUS * lpServiceArray = NULL;
        // Enumerate all services installed on current machine
        BOOL returnValue = EnumServicesStatus(
            hSvcCtlMgr,
            SERVICE_WIN32,
            SERVICE_STATE_ALL,
            NULL,
            0,
            &dwBytesNeeded,
            &dwServiceCount,
            &dwResumeHandle);

        if (!returnValue)
        {
            DWORD lastError = GetLastError();
            if ((lastError == ERROR_INSUFFICIENT_BUFFER) || (lastError == ERROR_MORE_DATA))
            {
                lpServiceArray = (ENUM_SERVICE_STATUS *)AllocateMemory(dwBytesNeeded);
                if (lpServiceArray == NULL)
                {
                    errorToReturn = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                dwResumeHandle = 0;
                returnValue = EnumServicesStatus(
                    hSvcCtlMgr,
                    SERVICE_WIN32,
                    SERVICE_STATE_ALL,
                    lpServiceArray,
                    dwBytesNeeded,
                    &dwBytesNeeded,
                    &dwServiceCount,
                    &dwResumeHandle);
                if (!returnValue)
                {
                    errorToReturn = GetLastError();
                    FreeMemory(lpServiceArray);
                    break;
                }
            }
            else
            {
                errorToReturn = lastError;
                break;
            }
        }
        else
        {
            errorToReturn = ERROR_INVALID_ENVIRONMENT;
            break;
        }

        // Take snapshot of all services, and compare the current status of each service
        // with previous status, then issue indication event if status was changed
        // since last polling.
        for(dwServiceIndex = 0; dwServiceIndex < dwServiceCount; dwServiceIndex++)
        {
            WindowsService * pService = NULL;
            BOOL found = FALSE;
            DWORD dwResult;

            // try to find the service in snapshot
            dwResult = FindAndAddIfNotFound(lpServiceArray[dwServiceIndex].lpServiceName, &found, &pService);
            if (dwResult != ERROR_SUCCESS)
            {
                continue;
            }
            else if (found && (pService != NULL))
            {
                // generate indication
                GenerateIndication(pArg, pService->dwState, &lpServiceArray[dwServiceIndex]);
            }
            // update the snapshot status of the service
            if (pService != NULL)
            {
                pService->dwState = lpServiceArray[dwServiceIndex].ServiceStatus.dwCurrentState;
            }
        }

        FreeMemory(lpServiceArray);

        // polling every POLLING_INTERVAL (ms), if stop event was signaled, then stop polling thread
        {
            DWORD dwWaitResult = WaitForSingleObject(g_hStopPollingEvent, POLLING_INTERVAL);
            switch(dwWaitResult)
            {
            case WAIT_OBJECT_0:
                // shutdown the polling thread
                ResetEvent(g_hStopPollingEvent);
                bShutdown = TRUE;
                break;
            default:
                break;
            }
        }
    }
    while (!bShutdown);

    CloseServiceHandle(hSvcCtlMgr);
    return errorToReturn;
}
