// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module:
//      atmevent.h
//
// Abstract:
//      Definitions and prototypes specific to this application
//
//

#ifndef ATMEVENT_H
#define ATMEVENT_H


#include <winsock2.h>
#include <ws2atm.h>
#include <strsafe.h>
#include <stdlib.h>
#include <stdio.h>

#define Malloc(x) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (x))
#define Free(x) HeapFree(GetProcessHeap(),0,(x))


// +1 included for string termination character
#define MAX_ATM_INTERFACE_LEN 41

typedef enum
{
    CMD_SENDER,
    CMD_RECEIVER,
    CMD_ENUMERATOR
} CMD_TYPE;


typedef struct _OPTIONS
{
    CMD_TYPE        cmd;
    CHAR            szLocalInterface[MAX_ATM_INTERFACE_LEN];
    CHAR            szRemoteInterface[MAX_ATM_INTERFACE_LEN];
    int             nRepeat;
    CHAR            fillchar;
    int             nBufSize;
    CHAR *          buf;
    DWORD           dwSleep;
    DWORD           dwSleepClose;
    DWORD           dwTotalClients;
    WSAPROTOCOL_INFO protocolInfo;
} OPTIONS;



extern VOID Receiver(
                    OPTIONS *pOptions);

extern VOID Sender(
                  OPTIONS *pOptions);

extern VOID Enumerator(
                      OPTIONS *pOptions);

extern void PrintQos(
                    QOS *qos, 
                    CHAR *lpszIndent);

#endif
