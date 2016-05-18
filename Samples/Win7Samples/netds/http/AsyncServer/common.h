//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __COMMON__
#define __COMMON__

#ifndef _WIN32_WINNT            
// Specifies that the minimum required platform is Windows Vista.
// Change this to the appropriate value to target other versions of Windows.
#define _WIN32_WINNT 0x0600     
#endif

#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4214)   // bit field types other than int

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <http.h>
#include <strsafe.h>

//
// Global defines
//

// Use the process heap for all memory allocation.
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

// Maximum string size (url, server directory)
#define MAX_STR_SIZE 256
// The number of requests for queueing
#define OUTSTANDING_REQUESTS 16
// The number of requests per processor
#define REQUESTS_PER_PROCESSOR 4
// This is the size of the buffer we provide to store the request.
// Headers, URL, entity-body, etc will all be stored in this buffer.
#define REQUEST_BUFFER_SIZE 4096

typedef VOID (*HTTP_COMPLETION_FUNCTION)(struct _HTTP_IO_CONTEXT*, PTP_IO, ULONG);

// Structure for handling http server context data
typedef struct _SERVER_CONTEXT
{
    // Server directory
    WCHAR wszRootDirectory[MAX_STR_SIZE];
    // Session Id
    HTTP_SERVER_SESSION_ID sessionId;
    // URL group
    HTTP_URL_GROUP_ID urlGroupId;
    // Request queue handle
    HANDLE hRequestQueue;
    // IO object
    PTP_IO Io;
    // TRUE, when the HTTP Server API driver was initialized
    BOOL bHttpInit;
    // TRUE, when we receive a user command to stop the server
    BOOL bStopServer;
} SERVER_CONTEXT, *PSERVER_CONTEXT;

// Structure for handling I/O context parameters
typedef struct _HTTP_IO_CONTEXT
{
    OVERLAPPED Overlapped;
    // Pointer to the completion function
    HTTP_COMPLETION_FUNCTION pfCompletionFunction;
    // Structure associated with the url and server directory
    PSERVER_CONTEXT pServerContext;
} HTTP_IO_CONTEXT, *PHTTP_IO_CONTEXT;

// Structure for handling I/O context parameters
typedef struct _HTTP_IO_REQUEST
{
    HTTP_IO_CONTEXT ioContext;
    PHTTP_REQUEST pHttpRequest;
    UCHAR RequestBuffer[REQUEST_BUFFER_SIZE];
} HTTP_IO_REQUEST, *PHTTP_IO_REQUEST;

// Structure for handling I/O context parameters
typedef struct _HTTP_IO_RESPONSE
{
    HTTP_IO_CONTEXT ioContext;

    // Structure associated with the specific response
    HTTP_RESPONSE HttpResponse;

    // Structure represents an individual block of data either in memory,
    // in a file, or in the HTTP Server API response-fragment cache.
    HTTP_DATA_CHUNK HttpDataChunk;
} HTTP_IO_RESPONSE, *PHTTP_IO_RESPONSE;

//
// Forward Prototypes
//

VOID SendCompletionCallback(
                            PHTTP_IO_CONTEXT pIoContext,
                            PTP_IO Io,
                            ULONG IoResult
                            );

VOID ReceiveCompletionCallback(
                               PHTTP_IO_CONTEXT pIoContext,
                               PTP_IO Io,
                               ULONG IoResult
                               );

VOID ProcessReceiveAndPostResponse(
                                   PHTTP_IO_REQUEST pIoRequest,
                                   PTP_IO Io,
                                   ULONG IoResult
                                   );

BOOL GetFilePathName(
                     PCWSTR BasePath, 
                     PCWSTR AbsPath, 
                     PWCHAR Buffer,
                     ULONG BufferSize
                     );

PHTTP_IO_REQUEST AllocateHttpIoRequest(
                                       PSERVER_CONTEXT pServerContext
                                       );

PHTTP_IO_RESPONSE AllocateHttpIoResponse(
                                         PSERVER_CONTEXT pServerContext
                                         );

VOID CleanupHttpIoResponse(
                           PHTTP_IO_RESPONSE pIoResponse
                           );

VOID CleanupHttpIoRequest(
                          PHTTP_IO_REQUEST pIoRequest
                          );
#endif
