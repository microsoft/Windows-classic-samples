// MonSvc.cpp : Sample service to illustrate usage of Notification APIs
// MonSvc monitors start and start of other services. 

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

//
// Globals
//
SERVICE_STATUS          ssService;
SERVICE_STATUS_HANDLE   hssService;
HANDLE                  g_hEvent;

//
// Structs
//
typedef struct _NOTIFY_CONTEXT
{
    PTSTR   pszServiceName;
} NOTIFY_CONTEXT, *PNOTIFY_CONTEXT;

// **************************************************************************
// Function Name: SvcDebugOut()
//
// Purpose: Output error message to the debugger. 
//
// Arguments:
//    IN LPTSTR String - Error message
//    IN DWORD Status - Error code
//
// Return Values:
//    None
VOID SvcDebugOut(LPTSTR String, DWORD Status);

// **************************************************************************
// Function Name: WriteMonitorLog()
//
// Purpose: Helper routine for logging service status changes 
//
// Arguments:
//    IN LPTSTR szStatus - Log message
//
// Return Values:
//    None
VOID WriteMonitorLog(LPTSTR szStatus);

// **************************************************************************
// Function Name: StartMonitor
//
// Purpose: Monitoring routine
//
// Return Values:
//    Win32 Exit code
DWORD StartMonitor (VOID);

// **************************************************************************
// Function Name: NotifyCallback
//
// Purpose: Invoked on service state change notification. 
//
// Arguments:
//    IN PVOID pParameter - Callback context
//
// Return Values:
//    None
VOID CALLBACK NotifyCallback (PVOID pParameter);

// **************************************************************************
// Function Name: ServiceStart
//
// Purpose: Service entry point
//
// Return Values:
//    None
VOID WINAPI ServiceStart(DWORD argc, LPTSTR *argv);

DWORD WINAPI ServiceCtrlHandler(DWORD Opcode, DWORD EventType, PVOID pEventData, PVOID pContext);

/****************************************************************************/

int __cdecl wmain (DWORD argc, LPWSTR argv[])
{
    SERVICE_TABLE_ENTRY   DispatchTable[] =
    {
        {TEXT("MonSvc"),  ServiceStart},
        {NULL,  NULL}
    };

    StartServiceCtrlDispatcher(DispatchTable);

    ExitProcess ( 0 );

    return 0;
}


VOID WINAPI ServiceStart(DWORD argc, LPTSTR *argv)
{
    HANDLE      hAutostartEvent = NULL;
    DWORD       dwError = ERROR_SUCCESS;

    hssService = RegisterServiceCtrlHandlerEx(TEXT("MonSvc"), ServiceCtrlHandler, NULL);

    if (hssService == (SERVICE_STATUS_HANDLE)0)
    {
        dwError = GetLastError();
        SvcDebugOut(TEXT("RegisterServiceCtrlHandler failed - "), dwError);
        ExitProcess(dwError);               
    }

    // Initialize the service status structure
    ssService.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    ssService.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;                                    
    ssService.dwServiceSpecificExitCode = 0;

    // Notification for stopping service.
    g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hEvent == NULL)
    {
        dwError = GetLastError();
        SvcDebugOut(TEXT("CreateEvent error"), dwError);

        ssService.dwCurrentState       = SERVICE_STOPPED;
        ssService.dwCheckPoint         = 0;
        ssService.dwWaitHint           = 0;
        ssService.dwWin32ExitCode      = dwError;

        if (!SetServiceStatus (hssService, &ssService))
        {
            SvcDebugOut(TEXT("SetServiceStatus error - "), GetLastError());
        }

        return;
    }

    //
    // Initialization complete - report running status. We start accepting stop and
    // shutdown controls only after reporting a status of SERVICE_RUNNING. It is easier
    // to design services that don't accept stop/shutdown controls while they are
    // in SERVICE_START_PENDING state.
    //

    ssService.dwCurrentState       = SERVICE_RUNNING;
    ssService.dwCheckPoint         = 0;
    ssService.dwWaitHint           = 0;
    ssService.dwWin32ExitCode      = 0;

    SvcDebugOut(TEXT("Initialized and running - "), 0);

    if (!SetServiceStatus (hssService, &ssService))
    {
        SvcDebugOut(TEXT("SetServiceStatus error - "), GetLastError());
    }

    dwError = StartMonitor();

    ssService.dwCurrentState       = SERVICE_STOPPED;
    ssService.dwCheckPoint         = 0;
    ssService.dwWaitHint           = 0;
    ssService.dwWin32ExitCode      = dwError;

    if (!SetServiceStatus (hssService, &ssService))
    {
        SvcDebugOut(TEXT("SetServiceStatus error - "), GetLastError());
    }

    return;
}


DWORD StartMonitor(VOID)
{
    HKEY hkOpenKey = NULL; 
    long lResult;
    WCHAR       szServiceParamKey[] = L"SYSTEM\\CurrentControlSet\\Services\\MonSvc\\Parameters";
    WCHAR       szParamValue[] = L"SvcName";
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       dwStatus;
    DWORD       dwMask;
    DWORD       dwBufSize;
    SC_HANDLE   hSCManager;
    SC_HANDLE   hService = NULL;
    LPTSTR      lpszServiceName;
    NOTIFY_CONTEXT NotifyContext = { 0 };
    SERVICE_NOTIFY  snServiceNotify;

    // Get svervice name from registry.  
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szServiceParamKey, 0, KEY_READ, &hkOpenKey);
    if(lResult != ERROR_SUCCESS)
    {
        dwError = GetLastError();
        SvcDebugOut(TEXT("Unable to open reg key - "), dwError);
        goto FnExit;         
    }

    lResult = RegQueryValueEx(hkOpenKey, szParamValue, NULL, NULL, NULL, &dwBufSize);
    if(lResult != ERROR_SUCCESS)
    {
        dwError = GetLastError();
        SvcDebugOut(TEXT("Unable to query value size - "), dwError);
        goto FnExit;                 
    }

    lpszServiceName = new TCHAR[dwBufSize/sizeof(TCHAR)];
    if(lpszServiceName == NULL)
    {
        dwError = GetLastError();
        SvcDebugOut(TEXT("Could not allocate memory - "), dwError);
        goto FnExit;
    }

    lResult = RegQueryValueEx(hkOpenKey, szParamValue, NULL, NULL, (LPBYTE) lpszServiceName, &dwBufSize);
    if(lResult != ERROR_SUCCESS)
    {
        dwError = GetLastError();
        SvcDebugOut(TEXT("Unable to query value - "), dwError);
        goto FnExit;                 
    }

    // Open SCM
    hSCManager = OpenSCManager (NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);

    if ( hSCManager == NULL )
    {
        dwError = GetLastError();
        SvcDebugOut(TEXT("Unable to open connection to SCM - "), dwError);
        goto FnExit;
    }

    // Open the target service
    hService = OpenService ( hSCManager, lpszServiceName, SERVICE_QUERY_STATUS);

    if ( hService == NULL )
    {
        dwError = GetLastError();
        SvcDebugOut(TEXT("OpenService failed - "), dwError);
        goto FnExit;
    }

    // Initialize callback context
    NotifyContext.pszServiceName = lpszServiceName;                 

    // Intialize notification struct
    snServiceNotify.dwVersion           =   SERVICE_NOTIFY_STATUS_CHANGE;
    snServiceNotify.pfnNotifyCallback   =   (PFN_SC_NOTIFY_CALLBACK) NotifyCallback;
    snServiceNotify.pContext =  &NotifyContext;

    // We care about changes to RUNNING and STOPPED states only
    dwMask = SERVICE_NOTIFY_RUNNING | SERVICE_NOTIFY_STOPPED;

    while(TRUE)
    {
        // Register for notification
        dwStatus = NotifyServiceStatusChange ( hService, dwMask, &snServiceNotify);

        if( dwStatus != ERROR_SUCCESS )
        {   
            SvcDebugOut(TEXT("NSSC failed - "), dwStatus);
            dwError = dwStatus;
            goto FnExit;
        }


        // Wait for notification to fire (or) for STOP control
        dwStatus = WaitForSingleObjectEx(g_hEvent, INFINITE, TRUE);

        // Check if this was signaled due to a SERVICE_STOP control
        if(dwStatus == WAIT_OBJECT_0)
        {
            break;
        }
    }

FnExit:
    if(hService)
    {
        CloseServiceHandle ( hService );
        hService = NULL;
    }

    if(hSCManager)
    {
        CloseServiceHandle (hSCManager);
        hSCManager = NULL;
    }

    if(hkOpenKey != NULL)
    {
        RegCloseKey(hkOpenKey);
    }

    delete[] lpszServiceName;           

    return dwError;
}


VOID CALLBACK NotifyCallback(PVOID pParameter)
{       
    HRESULT hr = S_OK;
    PSERVICE_NOTIFY         pNotify = ( PSERVICE_NOTIFY ) pParameter;
    PNOTIFY_CONTEXT         pContext = ( PNOTIFY_CONTEXT ) pNotify->pContext;
    TCHAR szStatus[1024];

    if(pNotify->ServiceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        hr = StringCchPrintf(szStatus, 1024, TEXT("%s %s.\r\n"), pContext->pszServiceName, TEXT("entered running state")); 
    }
    else
    {
        hr = StringCchPrintf(szStatus, 1024, TEXT("%s %s.\r\n"), pContext->pszServiceName, TEXT("entered stopped state")); 
    }

    if(hr != S_OK)
    {
        OutputDebugString(TEXT("Error creating status msg"));               
    }
    else
    {
        WriteMonitorLog(szStatus);
    }
}


DWORD WINAPI ServiceCtrlHandler(DWORD Opcode, DWORD EventType, PVOID pEventData, PVOID pContext)
{
    switch(Opcode)
    {
    case SERVICE_CONTROL_SHUTDOWN:
        SvcDebugOut(TEXT("Shutdown command received - "), Opcode);

        //
        // Fall through to STOP case
        //

    case SERVICE_CONTROL_STOP:
        ssService.dwWin32ExitCode = ERROR_SUCCESS;
        ssService.dwCurrentState  = SERVICE_STOP_PENDING;
        ssService.dwCheckPoint    = 1;
        ssService.dwWaitHint      = 10000;
        break;

    default:
        SvcDebugOut(TEXT("Unrecognized opcode - "), Opcode);
    }

    if (!SetServiceStatus(hssService, &ssService))
    {
        SvcDebugOut(TEXT("SetServiceStatus error - "), GetLastError());
    }

    // Notify the service thread
    if (Opcode == SERVICE_CONTROL_STOP || Opcode == SERVICE_CONTROL_SHUTDOWN) 
    {
        SetEvent(g_hEvent);
    }

    return ( ERROR_SUCCESS );
}


VOID SvcDebugOut(LPTSTR String, DWORD Status) 
{ 
    HRESULT hr = S_OK;
    TCHAR  Buffer[1024]; 
    hr = StringCchPrintf(Buffer, 1024, String, Status); 
    if(hr == S_OK)
    {
        OutputDebugString(Buffer); 
    }
    else
    {
        OutputDebugString(TEXT("Error in Dbg string")); 
    }
}


VOID WriteMonitorLog(LPTSTR szStatus) 
{
    HANDLE hLogFile = INVALID_HANDLE_VALUE;
    HRESULT hr = S_OK;
    LPCTSTR szFileNameSuffix = TEXT("\\MonSvc\\SvcMonitor.log");
    TCHAR szFileName[MAX_PATH];
    BOOL bRet = FALSE;
    DWORD dwSize;

    if(szStatus == NULL) 
    {
        SvcDebugOut(TEXT("Invalid service status - "), ERROR_INVALID_PARAMETER);
        goto FnExit;
    }   

    dwSize = ExpandEnvironmentStrings(TEXT("%ProgramFiles%"), szFileName, MAX_PATH);
    if(dwSize == 0 || dwSize > MAX_PATH)
    {
        SvcDebugOut(TEXT("File name too long - "), GetLastError());
        goto FnExit;       
    }

    hr = StringCchCat(szFileName, MAX_PATH, szFileNameSuffix);
    if(hr != S_OK)
    {
        SvcDebugOut(TEXT("File name too long - "), ERROR_INSUFFICIENT_BUFFER);
        goto FnExit;       
    }


    hLogFile = CreateFile(szFileName, 
        GENERIC_READ|GENERIC_WRITE, 
        0, 
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hLogFile == INVALID_HANDLE_VALUE) 
    {       
        SvcDebugOut(TEXT("Cannot open monitor log - "), GetLastError());
        goto FnExit;
    }

    SetFilePointer(hLogFile, 0, NULL, FILE_END);    
    bRet = WriteFile(hLogFile, szStatus,  DWORD ((_tcslen(szStatus)) * sizeof(TCHAR)), &dwSize, NULL);

    if(!bRet)
    {       
        SvcDebugOut(TEXT("Cannot write to  log - "), GetLastError());
    }

FnExit:
    CloseHandle(hLogFile);
}