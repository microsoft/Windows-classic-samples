/*++

Copyright (c) 1997-1999  Microsoft Corporation

Module Name:

    mspdebug.cpp

Abstract:

    This module contains the debugging support for the MSPs. 

--*/

#include "precomp.h"
#pragma hdrstop

#include <stdio.h>


//
// no need to build this code if MSPLOG is not defined
//

#ifdef MSPLOG


#define MAXDEBUGSTRINGLENGTH 512

static DWORD   sg_dwTraceID = INVALID_TRACEID;

static char    sg_szTraceName[100];   // saves name of dll
static DWORD   sg_dwTracingToDebugger = 0;
static DWORD   sg_dwTracingToConsole  = 0;
static DWORD   sg_dwTracingToFile     = 0;
static DWORD   sg_dwDebuggerMask      = 0;


//
// this flag indicates whether any tracing needs to be done at all
//

BOOL g_bMSPBaseTracingOn = FALSE;


BOOL NTAPI MSPLogRegister(LPCTSTR szName)
{
    HKEY       hTracingKey;

    char       szTracingKey[100];
    const char szDebuggerTracingEnableValue[] = "EnableDebuggerTracing";
    const char szConsoleTracingEnableValue[] = "EnableConsoleTracing";
    const char szFileTracingEnableValue[] = "EnableFileTracing";
    const char szTracingMaskValue[]   = "ConsoleTracingMask";


    sg_dwTracingToDebugger = 0;
    sg_dwTracingToConsole = 0;
    sg_dwTracingToFile = 0; 

#ifdef UNICODE
    sprintf_s(szTracingKey,sizeof(szTracingKey), "Software\\Microsoft\\Tracing\\%ls", szName);
#else
    sprintf_s(szTracingKey,sizeof(szTracingKey), "Software\\Microsoft\\Tracing\\%s", szName);
#endif

    _ASSERTE(sg_dwTraceID == INVALID_TRACEID);

    if ( ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        szTracingKey,
                                        0,
                                        KEY_READ,
                                        &hTracingKey) )
    {
        DWORD      dwDataSize = sizeof (DWORD);
        DWORD      dwDataType;

        RegQueryValueExA(hTracingKey,
                         szDebuggerTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToDebugger,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szConsoleTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToConsole,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szFileTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToFile,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szTracingMaskValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwDebuggerMask,
                         &dwDataSize);

        RegCloseKey (hTracingKey);
    }
    else
    {

        //
        // the key could not be opened. in case the key does not exist, 
        // register with rtutils so that the reg keys get created
        //

#ifdef UNICODE
        sprintf_s(sg_szTraceName,sizeof(sg_szTraceName), "%ls", szName);
#else
        sprintf_s(sg_szTraceName,sizeof(sg_szTraceName), "%s", szName);
#endif

        //
        // tracing should not have been initialized
        //
        
        sg_dwTraceID = TraceRegister(szName);

        if (sg_dwTraceID != INVALID_TRACEID)
        {
            g_bMSPBaseTracingOn = TRUE;
        }

        return TRUE;
    }


    //
    // are we asked to do any tracing at all?
    //
    
    if ( (0 != sg_dwTracingToDebugger) ||
         (0 != sg_dwTracingToConsole ) ||
         (0 != sg_dwTracingToFile    )    )
    {

        //
        // see if we need to register with rtutils
        //

        if ( (0 != sg_dwTracingToConsole ) || (0 != sg_dwTracingToFile) )
        {

            //
            // rtutils tracing is enabled. register with rtutils
            //


#ifdef UNICODE
            sprintf_s(sg_szTraceName,sizeof(sg_szTraceName), "%ls", szName);
#else
            sprintf_s(sg_szTraceName,sizeof(sg_szTraceName), "%s", szName);
#endif

            //
            // tracing should not have been initialized
            //

            _ASSERTE(sg_dwTraceID == INVALID_TRACEID);


            //
            // register
            //

            sg_dwTraceID = TraceRegister(szName);
        }


        //
        // if debugger tracing, or succeeded registering with rtutils, we are all set
        //

        if ( (0 != sg_dwTracingToDebugger) || (sg_dwTraceID != INVALID_TRACEID) )
        {

            //
            // some tracing is enabled. set the global flag
            //

            g_bMSPBaseTracingOn = TRUE;

            return TRUE;
        }
        else
        {


            //
            // registration did not go through and debugger logging is off 
            //

            return FALSE;
        }
    }

    
    //
    // logging is not enabled. nothing to do
    //

    return TRUE;
}

void NTAPI MSPLogDeRegister()
{
    if (g_bMSPBaseTracingOn)
    {
        sg_dwTracingToDebugger = 0;
        sg_dwTracingToConsole = 0;
        sg_dwTracingToFile = 0; 


        //
        // if we registered tracing, unregister now
        //

        if ( sg_dwTraceID != INVALID_TRACEID )
        {
            TraceDeregister(sg_dwTraceID);
            sg_dwTraceID = INVALID_TRACEID;
        }
    }
}


void NTAPI LogPrint(IN DWORD dwDbgLevel, IN LPCSTR lpszFormat, IN ...)
/*++

Routine Description:

    Formats the incoming debug message & calls TraceVprintfEx to print it.

Arguments:

    dwDbgLevel   - The type of the message.

    lpszFormat - printf-style format string, followed by appropriate
                 list of arguments

Return Value:

--*/
{
    static char * message[] = 
    {
        "ERROR", 
        "WARNING", 
        "INFO", 
        "TRACE", 
        "EVENT",
        "INVALID TRACE LEVEL"
    };

    char  szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];
    
    DWORD dwIndex;

    if ( ( sg_dwTracingToDebugger > 0 ) &&
         ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )
    {
        switch(dwDbgLevel)
        {
        case MSP_ERROR: dwIndex = 0; break;
        case MSP_WARN:  dwIndex = 1; break;
        case MSP_INFO:  dwIndex = 2; break;
        case MSP_TRACE: dwIndex = 3; break;
        case MSP_EVENT: dwIndex = 4; break;
        default:        dwIndex = 5; break;
        }

        // retrieve local time
        SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);

        sprintf_s(szTraceBuf,
		  sizeof(szTraceBuf),
                  "%s:[%02u:%02u:%02u.%03u,tid=%x:]%s: ",
                  sg_szTraceName,
                  SystemTime.wHour,
                  SystemTime.wMinute,
                  SystemTime.wSecond,
                  SystemTime.wMilliseconds,
                  GetCurrentThreadId(), 
                  message[dwIndex]);

        va_list ap;
        va_start(ap, lpszFormat);

        _vsnprintf_s(&szTraceBuf[lstrlenA(szTraceBuf)], 
            sizeof(&szTraceBuf[lstrlenA(szTraceBuf)]),
            MAXDEBUGSTRINGLENGTH - lstrlenA(szTraceBuf), 
            lpszFormat, 
            ap
            );

        strcat_s (szTraceBuf,sizeof(szTraceBuf), "\n");

        OutputDebugStringA (szTraceBuf);

        va_end(ap);
    }

    if (sg_dwTraceID != INVALID_TRACEID)
    {
        if ( ( sg_dwTracingToConsole > 0 ) || ( sg_dwTracingToFile > 0 ) )
        {
            switch(dwDbgLevel)
            {
            case MSP_ERROR: dwIndex = 0; break;
            case MSP_WARN:  dwIndex = 1; break; 
            case MSP_INFO:  dwIndex = 2; break;
            case MSP_TRACE: dwIndex = 3; break;
            case MSP_EVENT: dwIndex = 4; break;
            default:        dwIndex = 5; break;
            }

            sprintf_s(szTraceBuf,sizeof(szTraceBuf), "[%s] %s", message[dwIndex], lpszFormat);

            va_list arglist;
            va_start(arglist, lpszFormat);
            TraceVprintfExA(sg_dwTraceID, dwDbgLevel | TRACE_USE_MSEC, szTraceBuf, arglist);
            va_end(arglist);
        }
    }
}

#endif // MSPLOG

// eof
