/*++

Copyright 1998 - 2000 Microsoft Corporation

Module Name:

    tssysinfo.h

Abstract:

    This module contains the declarations for the functions used in the check_sd
    sample.  The declarations are grouped based on the modules they are located in.

Author:

    Frank Kim (franki)    18-Mar-99

--*/

#ifndef _TSSYSINFO_
#define _TSSYSINFO_

#define STRICT
#define _

//
// undefine this if you want to build for UNICODE
//
//#define UNICODE
//#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <wtsapi32.h>
#include <tchar.h>
#include <lmcons.h>

#ifdef TSDLL

#include <pchannel.h>
#include <cchannel.h>

#endif

//
// definitions
//
#define CHANNELNAME "sysinfo"
#define TSVERSIONINFO 0
#define TSMEMORYINFO  1
#define QUERY_INTERVAL 2000  // milliseconds

//
// definitions
//
typedef struct{
    char          szClientName[MAX_COMPUTERNAME_LENGTH+1];
    TCHAR         szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    TCHAR         szUserName[UNLEN+1];
    OSVERSIONINFO osvi;
    SYSTEM_INFO   si;
} TS_VERSION_INFO;

//
// GLOBAL variables
//
#ifdef TSDLL

HANDLE                ghThread;
HANDLE                ghWriteEvent;
HANDLE                ghAlertThread;
HANDLE                ghStopThread;
HANDLE                ghSynchCodeEvent;
LPHANDLE              gphChannel;
DWORD                 gdwOpenChannel;
PCHANNEL_ENTRY_POINTS gpEntryPoints;
DWORD                 gdwControlCode;
char                  gszClientName[MAX_COMPUTERNAME_LENGTH+1];

#else

HANDLE                ghEventEnd;

#endif

//
// declarations
//
void DisplayError(LPTSTR pszAPI);

#ifdef TSDLL
void WaitForTheWrite(HANDLE hEvent);
void GetMemoryInfo(void);
void GetVersionInfo(void);
void WINAPI WorkerThread(void);
void WINAPI VirtualChannelOpenEvent(DWORD openHandle, UINT event, LPVOID pdata, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags);
VOID VCAPITYPE VirtualChannelInitEventProc(LPVOID pInitHandle, UINT event, LPVOID pData, UINT dataLength);
BOOL VCAPITYPE VirtualChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints);
#else
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);
#endif

#endif // _TSSYSINFO_

