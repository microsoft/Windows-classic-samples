/*++

Copyright 1998 - 2000 Microsoft Corporation

Module Name:

    sysinf_c.c

Abstract:

    This file contains the code for the client-side DLL to demonstrate the
    Virtual Channels feature on Windows Terminal Services.

    The server-side app sends a request to the client DLL for one of two
    possible types of packet: TSVERSIONINFO or TSMEMORYINFO.  This wakes up a
    worker thread which writes the requested packet to the virtual channel.

    The client DLL cleans up when the client is terminated.  The worker thread
    is signaled to stop, if it doesn't stop within 5 seconds, it is terminated.

Author:

    Frank Kim (franki)    18-Mar-99

--*/

#define TSDLL

#include "sysinfo.h"


void DisplayError(LPTSTR pszAPI)
{
    int    i;
    LPVOID lpvMessageBuffer;
    TCHAR  buffer[256] = TEXT("");
    DWORD  lastErr;

    lastErr = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, lastErr,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                  (LPTSTR)&lpvMessageBuffer, 0, NULL);

    //
    //... now display this string
    //
    i = sprintf_s(buffer, 256,        TEXT("SYSINF_C ERROR: API        = %s.\n"), pszAPI);
    i += sprintf_s(buffer + i, 256-i, TEXT("                error code = %u.\n"), lastErr);
    sprintf_s(buffer + i, 256-i,      TEXT("                message    = %s.\n"), (LPTSTR)lpvMessageBuffer);

    OutputDebugString(buffer);

    //
    // Free the buffer allocated by the system
    //
    LocalFree(lpvMessageBuffer);
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
UNREFERENCED_PARAMETER(lpvReserved);
UNREFERENCED_PARAMETER(hinstDLL);

    switch(fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            OutputDebugString(TEXT("SYSINF_C: process attach\n"));
            break;

        case DLL_THREAD_ATTACH:
            OutputDebugString(TEXT("SYSINF_C: thread attach\n"));
            break;

        case DLL_THREAD_DETACH:
            OutputDebugString(TEXT("SYSINF_C: thread detach\n"));
            break;

        case DLL_PROCESS_DETACH:
            OutputDebugString(TEXT("SYSINF_C: process detach\n"));
            break;

        default:
            OutputDebugString(TEXT("SYSINF_C: Unknown reason in DllMain\n"));
            break;
    }
    return TRUE;
}


void WaitForTheWrite(HANDLE hEvent)
{
    OutputDebugString(TEXT("SYSINF_C: worker thread waiting for write to complete\n"));

    if (!WaitForSingleObject(hEvent, INFINITE) == WAIT_FAILED)
        DisplayError(TEXT("WaitForSingleObject"));
}


void GetMemoryInfo(void)
{
    DWORD        dwControlCode = TSMEMORYINFO;
    MEMORYSTATUS ms;
    UINT         ui;

    OutputDebugString(TEXT("SYSINF_C: worker thread get memory info\n"));

    GlobalMemoryStatus(&ms);

    //
    // write memory info to channel
    //
    ui = gpEntryPoints->pVirtualChannelWrite(gdwOpenChannel,
                                             (LPVOID)&ms,
                                             sizeof(ms),
                                             (LPVOID)&dwControlCode);
    if (ui != CHANNEL_RC_OK)
        OutputDebugString(TEXT("SYSINF_C: worker thread VirtualChannelWrite failed\n"));

    //
    // wait for the write to complete
    //
    WaitForTheWrite(ghWriteEvent);
}


void GetVersionInfo(void)
{
    TS_VERSION_INFO tsvi;
    OSVERSIONINFO   osvi;
    UINT            ui;
    DWORD           dwControlCode = TSVERSIONINFO;
    SYSTEM_INFO     si;
    TCHAR           szUserName[UNLEN+1];
    TCHAR           szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD           dwSize;

    OutputDebugString(TEXT("SYSINF_C: worker thread get version info\n"));

    //
    // obtain version info
    //
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&osvi))
        DisplayError(TEXT("GetVersionEx"));

    ZeroMemory(&tsvi, sizeof(tsvi));
    tsvi.osvi = osvi;

    //
    // get the client name
    //
    strcpy_s(tsvi.szClientName, strlen(gszClientName)+1, gszClientName);

    //
    // get the computer name
    //
    dwSize = sizeof(szComputerName);
    ZeroMemory(szComputerName, dwSize);

    if (!GetComputerName(szComputerName, &dwSize))
        DisplayError(TEXT("GetComputerName"));

    strcpy_s(tsvi.szComputerName, MAX_COMPUTERNAME_LENGTH, szComputerName);

    //
    // get system info
    //
    GetSystemInfo(&si);

    tsvi.si = si;

    //
    // get user name
    //
    dwSize = sizeof(szUserName);
    if (!GetUserName(szUserName, &dwSize))
        DisplayError(TEXT("GetUserName"));

    strcpy_s(tsvi.szUserName, UNLEN, szUserName);

    //
    // write memory info to channel
    //
    ui = gpEntryPoints->pVirtualChannelWrite(gdwOpenChannel,
                                             (LPVOID)&tsvi,
                                             sizeof(tsvi),
                                             (LPVOID)&dwControlCode);
    if (ui != CHANNEL_RC_OK)
        OutputDebugString(TEXT("SYSINF_C: worker thread VirtualChannelWrite failed\n"));

    //
    // wait for the write to complete
    //
    WaitForTheWrite(ghWriteEvent);
}


void WINAPI WorkerThread(void)
{
    DWORD  dw;
    DWORD  dwControlCode;
    HANDLE hEventConnect[2]; // 0 stop, 1 alert
    BOOL   bDone = FALSE;

    hEventConnect[0] = ghStopThread;
    hEventConnect[1] = ghAlertThread;

    while( !bDone ) {

        OutputDebugString(TEXT("SYSINF_C: worker thread waiting...\n"));

        dw = WaitForMultipleObjects(2, hEventConnect, FALSE, INFINITE);

        switch(dw)
        {
            case WAIT_FAILED:
            {
                DisplayError(TEXT("WaitForMultipleObjects"));
            }
            break;

            case WAIT_OBJECT_0:
            {
                bDone = TRUE;
            }
            break;

            case WAIT_OBJECT_0 + 1:
            {
                OutputDebugString(TEXT("SYSINF_C: worker thread wakeup\n"));

                dwControlCode = gdwControlCode;

                //
                // signal
                //
                if (!SetEvent(ghSynchCodeEvent))
                    DisplayError(TEXT("SetEvent"));

                if (dwControlCode == TSMEMORYINFO)
                    GetMemoryInfo();
                else
                    GetVersionInfo();
            }
            break;

            default:
            {
                OutputDebugString(TEXT("SYSINF_C: worker thread - unknown wait rc\n"));
            }
            break;
        }
    }

    OutputDebugString(TEXT("SYSINF_C: worker thread ending\n"));

    ExitThread(0);
}


void WINAPI VirtualChannelOpenEvent(DWORD openHandle, UINT event, LPVOID pdata, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
    LPDWORD pdwControlCode = (LPDWORD)pdata;

    UNREFERENCED_PARAMETER(openHandle);
    UNREFERENCED_PARAMETER(dataFlags);

    if (dataLength == totalLength){
        switch(event)
        {
            case CHANNEL_EVENT_DATA_RECEIVED:
            {
                //
                // initialize
                //
                gdwControlCode = *pdwControlCode;

                //
                // initiate a write
                //
                if (!SetEvent(ghAlertThread))
                    DisplayError(TEXT("SetEvent"));
                else
                    OutputDebugString(TEXT("SYSINF_C: signal to wake up thread\n"));

                //
                // wait for worker thread to read control
                //
                if (WaitForSingleObject(ghSynchCodeEvent, INFINITE) == WAIT_FAILED)
                    DisplayError(TEXT("WaitForSingleObject"));
            }
            break;

            case CHANNEL_EVENT_WRITE_COMPLETE:
            {
                switch (*pdwControlCode)
                {
                    case TSMEMORYINFO:
                        OutputDebugString(TEXT("SYSINF_C: write completed for memory info\n"));
                        break;

                    case TSVERSIONINFO:
                        OutputDebugString(TEXT("SYSINF_C: write completed for version info\n"));
                        break;

                    default:
                        OutputDebugString(TEXT("SYSINF_C: write completed for UNKNOWN\n"));
                        break;
                }

                //
                // set event to notify write is complete
                //
                if (!SetEvent(ghWriteEvent))
                    DisplayError(TEXT("SetEvent"));
            }
            break;

            case CHANNEL_EVENT_WRITE_CANCELLED:
            {
                //
                // set event to notify write is complete since write was cancelled
                //
                if (!SetEvent(ghWriteEvent))
                    DisplayError(TEXT("SetEvent"));
            }
            break;

            default:
            {
                OutputDebugString(TEXT("SYSINF_C: unrecognized open event\n"));
            }
            break;
        }
    }
    else
        OutputDebugString(TEXT("SYSINF_C: data is broken up\n"));
}


VOID VCAPITYPE VirtualChannelInitEventProc(LPVOID pInitHandle, UINT event, LPVOID pData, UINT dataLength)
{
    DWORD dwThreadId;
    UINT  ui;

    UNREFERENCED_PARAMETER(pInitHandle);
    UNREFERENCED_PARAMETER(dataLength);

    switch(event)
    {
        case CHANNEL_EVENT_INITIALIZED:
        {
            OutputDebugString(TEXT("SYSINF_C: channel initialized\n"));
        }
        break;

        case CHANNEL_EVENT_CONNECTED:
        {
            //
            // save client machine
            //
            ZeroMemory(gszClientName, sizeof(gszClientName));
            strcpy_s(gszClientName, strlen((LPSTR)pData)+1, (LPSTR)pData);

            OutputDebugString(TEXT("SYSINF_C: channel connected\n"));

            //
            // open channel
            //
            ui = gpEntryPoints->pVirtualChannelOpen(gphChannel,
                                                    &gdwOpenChannel,
                                                    CHANNELNAME,
                                                    (PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEvent);
            if (ui != CHANNEL_RC_OK){
                OutputDebugString(TEXT("SYSINF_C: virtual channel open failed\n"));
                return;
            }

            //
            // create write completion event
            //
            ghWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (ghWriteEvent == NULL)
                DisplayError(TEXT("CreateEvent"));

            //
            // create thread wakeup event
            //
            ghAlertThread = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (ghAlertThread == NULL)
                DisplayError(TEXT("CreateEvent"));

            //
            // create thread stop event
            //
            ghStopThread = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (ghStopThread == NULL)
                DisplayError(TEXT("CreateEvent"));

            //
            // create thread synch event
            //
            ghSynchCodeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (ghSynchCodeEvent == NULL)
                DisplayError(TEXT("CreateEvent"));

            //
            // spawn the worker thread
            //
            ghThread = CreateThread(NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)WorkerThread,
                                    NULL,
                                    0,
                                    &dwThreadId);
            if (ghThread == NULL)
                DisplayError(TEXT("CreateThread"));
        }
        break;

        case CHANNEL_EVENT_V1_CONNECTED:
        {
            OutputDebugString(TEXT("SYSINF_C: v1 connected\n"));
            MessageBox(NULL,
                       TEXT("Connecting to a non Windows 2000 Terminal Server"),
                       TEXT("sysinf_c.dll"),
                       MB_OK);
        }
        break;

        case CHANNEL_EVENT_DISCONNECTED:
        {
            DWORD dw;

            OutputDebugString(TEXT("SYSINF_C: disconnected\n"));

            //
            // signal worker thread to end
            //
            if (!SetEvent(ghStopThread))
                DisplayError(TEXT("SetEvent"));

            //
            // wait for thread to die
            //
            dw = WaitForSingleObject(ghThread, 5000);

            switch(dw)
            {
                case WAIT_TIMEOUT:
                    //
                    // blow away thread since time expired
                    //
                    OutputDebugString(TEXT("SYSINF_C: blowing away worker thread\n"));
                    if (!TerminateThread(ghThread, 0))
                        DisplayError(TEXT("TerminateThread"));
                    break;

                case WAIT_FAILED:
                    DisplayError(TEXT("WaitForSingleObject"));
                    break;
                default:
                    break;
            }

            OutputDebugString(TEXT("SYSINF_C: worker thread terminated\n"));

            //
            // close handles
            //
            CloseHandle(ghStopThread);
            CloseHandle(ghAlertThread);
            CloseHandle(ghWriteEvent);
            CloseHandle(ghSynchCodeEvent);
        }
        break;

        case CHANNEL_EVENT_TERMINATED:
        {
            OutputDebugString(TEXT("SYSINF_C: client terminated\n"));
            //
            // free the entry points table
            //
            LocalFree((HLOCAL)gpEntryPoints);
        }
        break;

        default:
        {
            OutputDebugString(TEXT("SYSINF_C: unrecognized init event\n"));
        }
        break;
    }
}


BOOL VCAPITYPE VirtualChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints)
{
    CHANNEL_DEF cd;
    UINT        uRet;

    OutputDebugString(TEXT("SYSINF_C: virtualchannelentry called\n"));

    //
    // allocate memory
    //
    gpEntryPoints = LocalAlloc(LPTR, pEntryPoints->cbSize);
    if (gpEntryPoints == NULL)
        DisplayError(TEXT("LocalAlloc"));

    memcpy(gpEntryPoints, pEntryPoints, pEntryPoints->cbSize);

    //
    // initialize global
    //
    //gpEntryPoints = pEntryPoints;

    //
    // initialize CHANNEL_DEF structure
    //
    ZeroMemory(&cd, sizeof(CHANNEL_DEF));
    strcpy_s(cd.name, strlen(CHANNELNAME)+1, CHANNELNAME); // ANSI ONLY

    //
    // register channel
    //
    uRet = gpEntryPoints->pVirtualChannelInit((LPVOID *)&gphChannel,
                                              (PCHANNEL_DEF)&cd, 1,
                                              VIRTUAL_CHANNEL_VERSION_WIN2000,
                                              (PCHANNEL_INIT_EVENT_FN)VirtualChannelInitEventProc);
    if (uRet != CHANNEL_RC_OK)
        return FALSE;

    //
    // make sure channel was initialized
    //
    if (cd.options != CHANNEL_OPTION_INITIALIZED)
        return FALSE;

    return TRUE;
}

