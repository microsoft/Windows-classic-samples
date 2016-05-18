//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------


#ifndef __COM_FAXFSP_SAMPLE

#define __COM_FAXFSP_SAMPLE

#include <windows.h>
#include <stdio.h>
#include <tapi.h>
#include <faxdev.h>
#include <winfax.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdlib.h>

#include "macros.h"
#include "reg.h"

// RESOURCE_STRING_LEN is the maximum length of a resource string
#define RESOURCE_STRING_LEN        256
// MAX_PATH_LEN is the maximum length of a fully-qualified path without the filename
#define MAX_PATH_LEN               MAX_PATH - 16

// NEWFSP_DEVICE_LIMIT is the virtual fax device limit of the newfsp service provider
#define NEWFSP_DEVICE_LIMIT        4

// NEWFSP_LOG_FILE is the name of the log file
#define NEWFSP_LOG_FILE            L"SampleFSP.log"

// DEVICE_NAME_PREFIX is the name prefix for the virtual fax devices
#define NEWFSP_DEVICE_NAME_PREFIX  L"SampleFSP Device "
// DEVICE_ID_PREFIX is the value that identifies the virtual fax devices
#define NEWFSP_DEVICE_ID_PREFIX    0x5000

// DEVICE_IDLE indicates the virtual fax device is idle
#define DEVICE_IDLE                1
// DEVICE_START indicates the virtual fax device is pending a fax job
#define DEVICE_START               2
// DEVICE_SEND indicates the virtual fax device is sending
#define DEVICE_SEND                3
// DEVICE_RECEIVE indicates the virtual fax device is receiving
#define DEVICE_RECEIVE             4
// DEVICE_ABORTING indicates the virtual fax device is aborting
#define DEVICE_ABORTING            5

// JOB_UNKNOWN indicates the fax job is pending
#define JOB_UNKNOWN                1
// JOB_SEND indicates the fax job is a send
#define JOB_SEND                   2
// JOB_RECEIVE indicates the fax job is a receive
#define JOB_RECEIVE                3

typedef struct _DEVICE_INFO {
    CRITICAL_SECTION     cs;                      // object to serialize access to the virtual fax device
    DWORD                DeviceId;                // specifies the identifier of the virtual fax device
    WCHAR                Directory[MAX_PATH_LEN]; // specifies the virtual fax device's incoming fax directory
    DWORD                Status;                  // specifies the current status of the virtual fax device
    HANDLE               ExitEvent;               // specifies the handle to the event to indicate the thread to watch for an incoming fax transmission is to exit
    struct _DEVICE_INFO  *pNextDeviceInfo;        // pointer to the next virtual fax device
    struct _JOB_INFO     *pJobInfo;               // pointer to the fax job associated with the virtual fax device
} DEVICE_INFO, *PDEVICE_INFO;

typedef struct _JOB_INFO {
    PDEVICE_INFO         pDeviceInfo;             // pointer to the virtual fax device data associated with the fax job
    HANDLE               CompletionPortHandle;    // specifies a handle to an I/O completion port
    ULONG_PTR            CompletionKey;           // specifies a completion port key value
    DWORD                JobType;                 // specifies the fax job type
    DWORD                Status;                  // specifies the current status of the fax job
    HLINE                LineHandle;              // specifies a handle to the open line device associated with the fax job
    HCALL                CallHandle;              // specifies a handle to the active call associated with the fax job
    LPWSTR               FileName;                // specifies the full path to the file that contains the data stream for the fax document
    LPWSTR               CallerName;              // specifies the name of the calling device
    LPWSTR               CallerNumber;            // specifies the telephone number of the calling device
    LPWSTR               ReceiverName;            // specifies the name of the receiving device
    LPWSTR               ReceiverNumber;          // specifies the telephone number of the receiving device
    DWORD                RetryCount;              // specifies the number of retries associated with the fax job
    BOOL                 Branding;                // specifies whether the fax service provider should generate a brand at the top of the fax transmission
    DWORD                PageCount;               // specifies the number of pages associated with the fax job
    LPWSTR               CSI;                     // specifies the identifier of the remote fax device
    LPWSTR               CallerId;                // specifies the identifier of the calling fax device
    LPWSTR               RoutingInfo;             // specifies the routing string associated with the fax job
} JOB_INFO, *PJOB_INFO;

static HANDLE        g_hInstance;       // g_hInstance is the global handle to the module
static HLINEAPP      g_LineAppHandle;   // g_LineAppHandle is the global handle to the fax service's registration with TAPI
static HANDLE        g_CompletionPort;  // g_CompletionPort is the global handle to an I/O completion port that the fax service provider must use to post I/O completion port packets to the fax service for asynchronous line status events
static ULONG_PTR     g_CompletionKey;   // g_CompletionKey is the global completion port key value

static HANDLE        g_hLogFile;        // g_hLogFile is the global handle to the log file
static PDEVICE_INFO  *g_pDeviceInfo;    // g_pDeviceInfo is the global pointer to the virtual fax device data

// Function definitions:

BOOL
GetNewFspRegistryData(
    BOOL          *bLoggingEnabled,
    LPWSTR        lpszLoggingDirectory,
    DWORD          dwLoggingDirectoryBufferSize,
    PDEVICE_INFO  *pDeviceInfo,
    LPDWORD       pdwNumDevices
);

VOID
SetNewFspRegistryData(
    BOOL          bLoggingEnabled,
    LPWSTR        lpszLoggingDirectory,
    PDEVICE_INFO  pDeviceInfo
);

BOOL
OpenLogFile(
    BOOL    bLoggingEnabled,
    LPWSTR  lpszLoggingDirectory
);

VOID
CloseLogFile(
);

VOID
WriteDebugString(
    LPWSTR  lpszFormatString,
    ...
);

VOID
PostJobStatus(
    HANDLE     CompletionPort,
    ULONG_PTR  CompletionKey,
    DWORD      StatusId,
    DWORD      ErrorCode
);

#endif