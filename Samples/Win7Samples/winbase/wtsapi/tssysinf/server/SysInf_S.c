/*++

Copyright 1998 - 2000 Microsoft Corporation

Module Name:

    sysinf_s.c

Abstract:

    This file contains the code for the server-side application to demonstrate
    Virtual Channels on Windows Terminal Services.

    The server application first obtains system information from the client
    machine and then obtains memory information, which is updated every 2
    seconds.

    The server app will stop obtaining memory information when the client
    disconnects and will resume when the client reconnects.

    The server app ends when the user presses Ctrl-C or Ctrl-Break.  The
    control handler will signal an event to stop obtaining memory info and
    clean up any extraneous handles.

Author:

    Frank Kim (franki)    18-Mar-99

--*/

#include "sysinfo.h"

#define TEXT1(x) x


void DisplayError(LPTSTR pszAPI)
{
    LPVOID lpvMessageBuffer;
    DWORD  lastErr;

    lastErr = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, lastErr,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                  (LPTSTR)&lpvMessageBuffer, 0, NULL);

    //
    //... now display this string
    //
    _tprintf(TEXT1("SYSINF_S ERROR: API        = %s.\n"), pszAPI);
    _tprintf(TEXT1("                error code = %d.\n"), lastErr);
    _tprintf(TEXT1("                message    = %s.\n"), (LPTSTR)lpvMessageBuffer);

    //
    // Free the buffer allocated by the system
    //
    LocalFree(lpvMessageBuffer);

    ExitProcess(GetLastError());
}


BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
    //
    // cleanup
    //

    if (!SetEvent(ghEventEnd))
        DisplayError(TEXT("SetEvent"));

    return TRUE;
}


void
_cdecl
_tmain(void)
{
    BOOL            bContinue = TRUE;
    BOOL            bQuery = TRUE;
    DWORD           dwTimeout = QUERY_INTERVAL;
    DWORD           dwWait = WAIT_TIMEOUT;
    DWORD           dwSessionId;
    DWORD           dwControlCode;
    ULONG           ulBytesWritten;
    ULONG           ulBytesRead;
    HANDLE          hVirtChannel;
    HANDLE          hEventConnect[3]; // 0 reconnect, 1 disconnect, 2 stop
    MEMORYSTATUS    ms;
    TS_VERSION_INFO tsvi;
    TCHAR           szEventReconnect[256] = TEXT("");
    TCHAR           szEventDisconnect[256] = TEXT("");
    TCHAR           buffer[256] = TEXT("");

    //
    // install console control handler
    //
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine, TRUE))
        DisplayError(TEXT("SetConsoleControlHandler"));

    //
    // obtain session id
    //
    if (!ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId))
        DisplayError(TEXT("ProcessIdToSessionId"));

    _stprintf_s(szEventReconnect, 256, TEXT("Global\\sysinf_s-%u-Reconnect"), dwSessionId);
    _stprintf_s(szEventDisconnect, 256, TEXT("Global\\sysinf_s-%u-Disconnect"), dwSessionId);

    //
    // exit if running in console session
    //
    if (dwSessionId == 0){
        _tprintf(TEXT1("running in console session\n"));
        return;
    }

    //
    // create connection events
    //
    hEventConnect[0] = CreateEvent(NULL, FALSE, FALSE, szEventReconnect);
    if (hEventConnect[0] == NULL)
        DisplayError(TEXT("CreateEvent"));

    hEventConnect[1] = CreateEvent(NULL, FALSE, FALSE, szEventDisconnect);
    if (hEventConnect[1] == NULL)
        DisplayError(TEXT("CreateEvent"));

    //
    // create stop event
    //
    ghEventEnd = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (ghEventEnd == NULL)
        DisplayError(TEXT("CreateEvent"));

    hEventConnect[2] = ghEventEnd;

    //
    // open virtual channel
    //
    hVirtChannel = WTSVirtualChannelOpen(WTS_CURRENT_SERVER_HANDLE,
                                         dwSessionId,
                                         CHANNELNAME);
    if (hVirtChannel == NULL)
        DisplayError(TEXT("WTSVirtualChannelOpen"));

    //
    // get system information
    //
    dwControlCode = TSVERSIONINFO;
    if (!WTSVirtualChannelWrite(hVirtChannel,
                                (PCHAR)&dwControlCode,
                                sizeof(dwControlCode),
                                &ulBytesWritten))
        DisplayError(TEXT("WTSVirtualChannelWrite"));

    //
    // read system info
    //
    if (!WTSVirtualChannelRead(hVirtChannel,
                               INFINITE,
                               (PCHAR)&tsvi,
                               sizeof(tsvi),
                               &ulBytesRead))
        DisplayError(TEXT("WTSVirtualChannelRead"));

    //
    // display system info
    //
    _tprintf(TEXT1("Client machine  : %s\n"), tsvi.szComputerName);
    printf("TS machine      : %s\n", tsvi.szClientName);
    _tprintf(TEXT1("User context    : %s\n"), tsvi.szUserName);
    _tprintf(TEXT1("Operating System: "));
    if (tsvi.osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        _tprintf(TEXT1("Windows 9x\n"));
    else if (tsvi.osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
        _tprintf(TEXT1("Windows NT or Windows 2000\n"));
    else
        _tprintf(TEXT1("Win32s\n"));

    _tprintf(TEXT1("OS Version      : %u.%u\n"), tsvi.osvi.dwMajorVersion, tsvi.osvi.dwMinorVersion);
    _tprintf(TEXT1("Build Number    : %u\n"), tsvi.osvi.dwBuildNumber);
    _tprintf(TEXT1("Service Pack    : %s\n"), tsvi.osvi.szCSDVersion);
    _tprintf(TEXT1("Page Size       : %u\n"), tsvi.si.dwPageSize);
    _tprintf(TEXT1("Number of procs : %u\n"), tsvi.si.dwNumberOfProcessors);

    if (tsvi.si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL){

        _tprintf(TEXT1("Processor type  : "), tsvi.si.dwNumberOfProcessors);
        switch (tsvi.si.wProcessorLevel)
        {
            case 3:
                _tprintf(TEXT1("Intel 80386\n"));
                break;

            case 4:
                _tprintf(TEXT1("Intel 80486\n"));
                break;

            case 5:
                _tprintf(TEXT1("Intel Pentium\n"));
                break;

            case 6:
                _tprintf(TEXT1("Intel Pentium Pro or Pentium II\n"));
                break;

            default:
                _tprintf(TEXT1("Intel ????\n"));
                break;
        }
    }

    _tprintf(TEXT1("\nMemory Load - Avail Physical - Avail Page File - Avail Virtual\n"));

    while(bContinue){
        dwWait = WaitForMultipleObjects(3, hEventConnect, FALSE, dwTimeout);
        switch(dwWait)
        {
            case WAIT_TIMEOUT:
                if (bQuery) {
                    //
                    // initialize buffer
                    //
                    ZeroMemory(&ms, sizeof(ms));
                    ulBytesRead = 0;
                    ZeroMemory(buffer, sizeof(buffer));

                    dwControlCode = TSMEMORYINFO;
                    if (!WTSVirtualChannelWrite(hVirtChannel,
                                                (PCHAR)&dwControlCode,
                                                sizeof(dwControlCode),
                                                &ulBytesWritten))
                        DisplayError(TEXT("WTSVirtualChannelWrite"));

                    //
                    // read data
                    //
                    if (!WTSVirtualChannelRead(hVirtChannel,
                                               INFINITE,
                                               (PCHAR)&ms,
                                               sizeof(ms),
                                               &ulBytesRead))
                        DisplayError(TEXT("WTSVirtualChannelRead"));

                    //
                    // display data
                    //
                    _stprintf_s(buffer, 256,
                             TEXT("%10lu%%   %14lu   %15lu   %13lu"),
                             ms.dwMemoryLoad,
                             ms.dwAvailPhys,
                             ms.dwAvailPageFile,
                             ms.dwAvailVirtual);
                    _tprintf(TEXT1("\r%s"), buffer);
                }
                break;

            case WAIT_OBJECT_0: // reconnect
                //
                // allow query to occur
                //
                bQuery = TRUE;
                dwTimeout = QUERY_INTERVAL;
                _tprintf(TEXT1("-reconnected-\n"), buffer);
                break;

            case WAIT_OBJECT_0 + 1: // disconnect
                //
                // do not query
                //
                bQuery = FALSE;
                dwTimeout = INFINITE;
                _tprintf(TEXT1("\n-disconnected-\n"), buffer);
                break;

            case WAIT_OBJECT_0 + 2: // stop
                bContinue = FALSE;
                break;

            case WAIT_FAILED:
                DisplayError(TEXT("WaitForMultipleObjects"));
                break;

            default:
                break;
        }
    }

    //
    // close the virtual channel
    //
    WTSVirtualChannelClose(hVirtChannel);

    //
    // close handles
    //
    CloseHandle(hEventConnect[0]);
    CloseHandle(hEventConnect[1]);
    CloseHandle(ghEventEnd);
}

