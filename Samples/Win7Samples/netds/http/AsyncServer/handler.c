//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "common.h"

//
// Global variables
//

static USHORT g_usOKCode = 200;
static CHAR g_szOKReason[] = "OK";

static USHORT g_usFileNotFoundCode = 404;
static CHAR g_szFileNotFoundReason[] = "Not Found";
static CHAR g_szFileNotFoundMessage[] = "File not found";
static CHAR g_szFileNotAccessibleMessage[] = "File could not be opened";
static CHAR g_szBadPathMessage[] = "Bad path";

static USHORT g_usBadRequestReasonCode = 400;
static CHAR g_szBadRequestReason[] = "Bad Request";
static CHAR g_szBadRequestMessage[] = "Bad request";

static USHORT g_usNotImplementedCode = 501;
static CHAR g_szNotImplementedReason[] = "Not Implemented";
static CHAR g_szNotImplementedMessage[] = "Server only supports GET";

static USHORT g_usEntityTooLargeCode = 413;
static CHAR g_szEntityTooLargeReason[] = "Request Entity Too Large";
static CHAR g_szEntityTooLargeMessage[] = "Large buffer support is not implemented";

//
// Routine Description:
//
//     Retrieves the next available HTTP request from the specified request 
//     queue asynchronously. If HttpReceiveHttpRequest call failed inline checks 
//     the reason and cancels the Io if necessary. If our attempt to receive 
//     an HTTP Request failed with ERROR_MORE_DATA the client is misbehaving 
//     and we should return it error 400 back. Pretend that the call 
//     failed asynchronously.
// 
// Arguments:
// 
//     pServerContext - context for the server
//
//     Io - Structure that defines the I/O object.
// 
// Return Value:
// 
//     N/A
// 

VOID PostNewReceive(
                    PSERVER_CONTEXT pServerContext,
                    PTP_IO Io
                    )
{
    PHTTP_IO_REQUEST pIoRequest;
    ULONG Result;

    pIoRequest = AllocateHttpIoRequest(pServerContext);

    if (pIoRequest == NULL)
        return;

    StartThreadpoolIo(Io);

    Result = HttpReceiveHttpRequest(
        pServerContext->hRequestQueue, 
        HTTP_NULL_ID, 
        HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY, 
        pIoRequest->pHttpRequest,
        sizeof(pIoRequest->RequestBuffer),
        NULL,
        &pIoRequest->ioContext.Overlapped
        );

    if (Result != ERROR_IO_PENDING &&
        Result != NO_ERROR)
    {    
        CancelThreadpoolIo(Io);

        fprintf(stderr, "HttpReceiveHttpRequest failed, error 0x%lx\n", Result);

        if (Result == ERROR_MORE_DATA)
        {
            ProcessReceiveAndPostResponse(pIoRequest, Io, ERROR_MORE_DATA);
        }
 
         CleanupHttpIoRequest(pIoRequest);
    }
}
 
//
// Routine Description:
//
//     Completion routine for the asynchronous HttpSendHttpResponse
//     call. This sample doesn't process the results of its send operations.
// 
// Arguments:
// 
//     IoContext - The HTTP_IO_CONTEXT tracking this operation.
//
//     Io - Ignored
// 
//     IoResult - Ignored
// 
// Return Value:
// 
//     N/A
// 

VOID SendCompletionCallback(
                            PHTTP_IO_CONTEXT pIoContext,
                            PTP_IO Io,
                            ULONG IoResult
                            )
{
    PHTTP_IO_RESPONSE pIoResponse;

    UNREFERENCED_PARAMETER(IoResult);
    UNREFERENCED_PARAMETER(Io);

    pIoResponse = CONTAINING_RECORD(pIoContext, 
                                    HTTP_IO_RESPONSE, 
                                    ioContext);

    CleanupHttpIoResponse(pIoResponse);
}

//
// Routine Description:
//
//     Creates a response for a successful get, the content is served
//     from a file.
// 
// Arguments:
// 
//     pServerContext - Pointer to the http server context structure.
//
//     hFile - Handle to the specified file.
//
// Return Value:
// 
//     Return a pointer to the HTTP_IO_RESPONSE structure.
//

PHTTP_IO_RESPONSE CreateFileResponse(
                                     PSERVER_CONTEXT pServerContext, 
                                     HANDLE hFile
                                     )
{
    PHTTP_IO_RESPONSE pIoResponse;
    PHTTP_DATA_CHUNK pChunk;

    pIoResponse = AllocateHttpIoResponse(pServerContext);

    if (pIoResponse == NULL)
        return NULL;
    
    pIoResponse->HttpResponse.StatusCode = g_usOKCode;
    pIoResponse->HttpResponse.pReason = g_szOKReason;
    pIoResponse->HttpResponse.ReasonLength = (USHORT)strlen(g_szOKReason);

    pChunk = &pIoResponse->HttpResponse.pEntityChunks[0];
    pChunk->DataChunkType = HttpDataChunkFromFileHandle;
    pChunk->FromFileHandle.ByteRange.Length.QuadPart = HTTP_BYTE_RANGE_TO_EOF;
    pChunk->FromFileHandle.ByteRange.StartingOffset.QuadPart = 0;
    pChunk->FromFileHandle.FileHandle = hFile;

    return pIoResponse;
}

//
// Routine Description:
//
//     Creates an http response if the requested file was not found.
// 
// Arguments:
// 
//     pServerContext - Pointer to the http server context structure.
//
//     code - The error code to use in the response
//
//     pReason - The reason string to send back to the client
//
//     pMessage - The more verbose message to send back to the client
// 
// Return Value:
// 
//     Return a pointer to the HTTP_IO_RESPONSE structure
//

PHTTP_IO_RESPONSE CreateMessageResponse(
                                        PSERVER_CONTEXT pServerContext,
                                        USHORT code,
                                        PCHAR pReason,
                                        PCHAR pMessage
                                        )
{
    PHTTP_IO_RESPONSE pIoResponse;
    PHTTP_DATA_CHUNK pChunk;

    pIoResponse = AllocateHttpIoResponse(pServerContext);

    if (pIoResponse == NULL)
        return NULL;

    // Can not find the requested file
    pIoResponse->HttpResponse.StatusCode = code;
    pIoResponse->HttpResponse.pReason = pReason;
    pIoResponse->HttpResponse.ReasonLength = (USHORT)strlen(pReason);

    pChunk = &pIoResponse->HttpResponse.pEntityChunks[0];
    pChunk->DataChunkType = HttpDataChunkFromMemory;
    pChunk->FromMemory.pBuffer = pMessage;
    pChunk->FromMemory.BufferLength = (ULONG)strlen(pMessage);

    return pIoResponse;
}

//
// Routine Description:
//
//     This routine processes the received request, builds an HTTP response,
//     and sends it using HttpSendHttpResponse.
//
// Arguments:
// 
//     IoContext - The HTTP_IO_CONTEXT tracking this operation.
//
//     Io - Structure that defines the I/O object.
// 
//     IoResult - The result of the I/O operation. If the I/O is successful,
//         this parameter is NO_ERROR. Otherwise, this parameter is one of
//         the system error codes.
// 
// Return Value:
// 
//     N/A
//

VOID ProcessReceiveAndPostResponse(
                                   PHTTP_IO_REQUEST pIoRequest,
                                   PTP_IO Io,
                                   ULONG IoResult
                                   )
{
    ULONG Result;
    HANDLE hFile;
    HTTP_CACHE_POLICY CachePolicy;
    PHTTP_IO_RESPONSE pIoResponse;
    PSERVER_CONTEXT pServerContext;

    pServerContext = pIoRequest->ioContext.pServerContext;
    hFile = INVALID_HANDLE_VALUE;

    switch(IoResult){
        case NO_ERROR:
        {
            WCHAR wszFilePath[MAX_STR_SIZE];       
            BOOL bValidUrl; 
  
            if (pIoRequest->pHttpRequest->Verb != HttpVerbGET){
                pIoResponse = CreateMessageResponse(
                                pServerContext,
                                g_usNotImplementedCode,
                                g_szNotImplementedReason,
                                g_szNotImplementedMessage);
                break;
            }
           
            bValidUrl = GetFilePathName(
                pServerContext->wszRootDirectory,
                pIoRequest->pHttpRequest->CookedUrl.pAbsPath, 
                wszFilePath, 
                MAX_STR_SIZE);

            if (bValidUrl == FALSE)
            {
                pIoResponse = CreateMessageResponse(
                                pServerContext,
                                g_usFileNotFoundCode,
                                g_szFileNotFoundReason,
                                g_szBadPathMessage);
                break;
            }
        
            hFile = CreateFileW(
                wszFilePath, 
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, 
                NULL);
    
            if (hFile == INVALID_HANDLE_VALUE)
            {
                if (GetLastError() == ERROR_PATH_NOT_FOUND || 
                    GetLastError() == ERROR_FILE_NOT_FOUND)
                {
                    pIoResponse = CreateMessageResponse(
                                    pServerContext,
                                    g_usFileNotFoundCode,
                                    g_szFileNotFoundReason,
                                    g_szFileNotFoundMessage);
                    break;
                }

                pIoResponse = CreateMessageResponse(
                                pServerContext,
                                g_usFileNotFoundCode,
                                g_szFileNotFoundReason,
                                g_szFileNotAccessibleMessage);
                break;
            }
        
            pIoResponse = CreateFileResponse(pServerContext, hFile);

            CachePolicy.Policy = HttpCachePolicyUserInvalidates;
            CachePolicy.SecondsToLive = 0;
            break;
        }
        case ERROR_MORE_DATA:
        {
            pIoResponse = CreateMessageResponse(
                            pServerContext,
                            g_usEntityTooLargeCode,
                            g_szEntityTooLargeReason,
                            g_szEntityTooLargeMessage);
            break;
        }
        default:
            // If the HttpReceiveHttpRequest call failed asynchronously
            // with a different error than ERROR_MORE_DATA, the error is fatal
            // There's nothing this function can do
            return;
    }

    if (pIoResponse == NULL)
    {
        return;
    }

    StartThreadpoolIo(Io);

    Result = HttpSendHttpResponse(
        pServerContext->hRequestQueue, 
        pIoRequest->pHttpRequest->RequestId,
        0,
        &pIoResponse->HttpResponse,
        (hFile != INVALID_HANDLE_VALUE) ? &CachePolicy : NULL,
        NULL,
        NULL,
        0,
        &pIoResponse->ioContext.Overlapped,
        NULL
        );

    if (Result != NO_ERROR &&
        Result != ERROR_IO_PENDING)
    {
        CancelThreadpoolIo(Io);

        fprintf(stderr, "HttpSendHttpResponse failed, error 0x%lx\n", Result);

        CleanupHttpIoResponse(pIoResponse);
    }
}

//
// Routine Description:
//
//     Completion routine for the asynchronous HttpReceiveHttpRequest
//     call. Check if the user asked us to stop the server. If not, send a 
//     response and post a new receive to HTTPAPI.
// 
// Arguments:
// 
//     IoContext - The HTTP_IO_CONTEXT tracking this operation.
//
//     Io - Structure that defines the I/O object.
// 
//     IoResult - The result of the I/O operation. If the I/O is successful,
//         this parameter is NO_ERROR. Otherwise, this parameter is one of
//         the system error codes.
// 
// Return Value:
// 
//     N/A
//

VOID ReceiveCompletionCallback(
                               PHTTP_IO_CONTEXT pIoContext,
                               PTP_IO Io,
                               ULONG IoResult
                               )
{
    PHTTP_IO_REQUEST pIoRequest;
    PSERVER_CONTEXT pServerContext;

    pIoRequest = CONTAINING_RECORD(pIoContext, 
                                   HTTP_IO_REQUEST, 
                                   ioContext);

    pServerContext = pIoRequest->ioContext.pServerContext;

    if (pServerContext->bStopServer == FALSE)
    {
        ProcessReceiveAndPostResponse(pIoRequest, Io, IoResult);
 
        PostNewReceive(pServerContext, Io);
    }
   
    CleanupHttpIoRequest(pIoRequest);
}
