// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// Copyright (c) Microsoft Corporation. All rights reserved
// 
// Abstract:
// 
//     This sample demonstrates how to create a simple HTTP server using the
//     HTTP API, v2. It does this using the system thread pool. 
// 
//     Threads within the thread pool receives I/O completions from the 
//     specified HTTPAPI request queue. They process these by calling the 
//     callback function according to the I/O context. As an example, we
//     send back an HTTP response to the specified HTTP request. If the request
//     was valid, the response will include the content of a file as the entity
//     body.
// 
//     Once compiled, to use this sample you would:
//
//     httpasyncserverapp <Url> <ServerDirectory>
//
//     where:
//
//     <Url>             is the Url base this sample will listen for.
//     <ServerDirectory> is the local directory to map incoming requested Url
//                       to locally.
//

#include "common.h"

HTTPAPI_VERSION g_HttpApiVersion = HTTPAPI_VERSION_2;

// 
// Routine Description:
// 
//     Allocates an HTTP_IO_REQUEST block, initializes some members 
//     of this structure and increments the I/O counter.
// 
// Arguments:
// 
//     pServerContext - Pointer to the http server context structure.
// 
// Return Value:
// 
//     Returns a pointer to the newly initialized HTTP_IO_REQUEST.
//     NULL upon failure.
//

PHTTP_IO_REQUEST AllocateHttpIoRequest(
                                       PSERVER_CONTEXT pServerContext
                                       )
{
    PHTTP_IO_REQUEST pIoRequest;

    pIoRequest = (PHTTP_IO_REQUEST)MALLOC(sizeof(HTTP_IO_REQUEST));

    if (pIoRequest == NULL)
        return NULL;

    ZeroMemory(pIoRequest, sizeof(HTTP_IO_REQUEST));

    pIoRequest->ioContext.pServerContext = pServerContext;
    pIoRequest->ioContext.pfCompletionFunction = ReceiveCompletionCallback;
    pIoRequest->pHttpRequest = (PHTTP_REQUEST) pIoRequest->RequestBuffer;

    return pIoRequest;
}

// 
// Routine Description:
// 
//     Allocates an HTTP_IO_RESPONSE block, setups a couple HTTP_RESPONSE members 
//     for the response function, gives them 1 EntityChunk, which has a default 
//     buffer if needed and increments the I/O counter.
// 
// Arguments:
// 
//     pServerContext - Pointer to the http server context structure.
// 
// Return Value:
// 
//     Returns a pointer to the newly initialized HTTP_IO_RESPONSE.
//     NULL upon failure.
// 

PHTTP_IO_RESPONSE AllocateHttpIoResponse(
                                         PSERVER_CONTEXT pServerContext
                                         )
{
    PHTTP_IO_RESPONSE pIoResponse;
    PHTTP_KNOWN_HEADER pContentTypeHeader;

    pIoResponse = (PHTTP_IO_RESPONSE)MALLOC(sizeof(HTTP_IO_RESPONSE));

    if (pIoResponse == NULL)
        return NULL;

    ZeroMemory(pIoResponse, sizeof(HTTP_IO_RESPONSE));

    pIoResponse->ioContext.pServerContext = pServerContext;
    pIoResponse->ioContext.pfCompletionFunction = SendCompletionCallback;

    pIoResponse->HttpResponse.EntityChunkCount = 1;
    pIoResponse->HttpResponse.pEntityChunks = &pIoResponse->HttpDataChunk;

    pContentTypeHeader = 
        &pIoResponse->HttpResponse.Headers.KnownHeaders[HttpHeaderContentType];
    pContentTypeHeader->pRawValue = "text/html";
    pContentTypeHeader->RawValueLength = 
        (USHORT)strlen(pContentTypeHeader->pRawValue);

    return pIoResponse;
}

// 
// Routine Description:
// 
//     Cleans the structure associated with the specific response.
//     Releases this structure, and decrements the I/O counter.
// 
// Arguments:
// 
//     pIoResponse - Pointer to the structure associated with the specific 
//                   response.
// 
// Return Value:
// 
//     N/A
// 

VOID CleanupHttpIoResponse(
                           PHTTP_IO_RESPONSE pIoResponse
                           )
{
    DWORD i;

    for (i = 0; i < pIoResponse->HttpResponse.EntityChunkCount; ++i)
    {
        PHTTP_DATA_CHUNK pDataChunk;
        pDataChunk = &pIoResponse->HttpResponse.pEntityChunks[i];

        if (pDataChunk->DataChunkType == HttpDataChunkFromFileHandle)
        {
            if (pDataChunk->FromFileHandle.FileHandle != NULL)
            {
                CloseHandle(pDataChunk->FromFileHandle.FileHandle);
                pDataChunk->FromFileHandle.FileHandle = NULL;
            }
        }
    }

    FREE(pIoResponse);
}

// 
// Routine Description:
// 
//     Cleans the structure associated with the specific request.
//     Releases this structure and decrements the I/O counter
// 
// Arguments:
// 
//     pIoRequest - Pointer to the structure associated with the specific request.
// 
// Return Value:
// 
//     N/A
// 

VOID CleanupHttpIoRequest(
                          PHTTP_IO_REQUEST pIoRequest
                          )
{
    FREE(pIoRequest);
}

// 
// Routine Description:
// 
//     Computes the full path filename given the requested Url. 
//     Takes the base path and add the portion of the client request
//     Url that comes after the base Url.
// 
// Arguments:
// 
//     pServerContext - The server we are associated with.
//
//     RelativePath - the client request Url that comes after the base Url.
// 
//     Buffer - Output buffer where the full path filename will be written.
// 
//     BufferSize - Size of the Buffer in bytes.
// 
// Return Value:
// 
//     TRUE - Success.
//     FALSE - Failure. Most likely because the requested Url did not
//         match the expected Url.
// 

BOOL GetFilePathName(
                     PCWSTR BasePath, 
                     PCWSTR RelativePath, 
                     PWCHAR Buffer,
                     ULONG BufferSize
                     )
{
    if (FAILED(StringCbCopyW(Buffer, 
                             BufferSize, 
                             BasePath)))
        return FALSE;

    if (FAILED(StringCbCatW(Buffer, 
                            BufferSize, 
                            RelativePath)))
        return FALSE;

    return TRUE;
}

// 
// Routine Description:
// 
//     The callback function to be called each time an overlapped I/O operation 
//     completes on the file. This callback is invoked by the system threadpool.
//     Calls the corresponding I/O completion function.
//
// 
// Arguments:
// 
//     Instance - Ignored.
//
//     pContext - Ignored.
// 
//     Overlapped  - A pointer to a variable that receives the address of the 
//                   OVERLAPPED structure that was specified when the 
//                   completed I/O operation was started.
// 
//     IoResult - The result of the I/O operation. If the I/O is successful, 
//                this parameter is NO_ERROR. Otherwise, this parameter is 
//                one of the system error codes.
// 
//     NumberOfBytesTransferred - Ignored.
// 
//     Io - A TP_IO structure that defines the I/O completion object that 
//          generated the callback.
// 
// Return Value:
// 
//     N/A
// 

VOID CALLBACK IoCompletionCallback(
                                   PTP_CALLBACK_INSTANCE Instance,
                                   PVOID pContext,
                                   PVOID pOverlapped,
                                   ULONG IoResult,
                                   ULONG_PTR NumberOfBytesTransferred,
                                   PTP_IO Io
                                   )
{
    PSERVER_CONTEXT pServerContext;

    UNREFERENCED_PARAMETER(NumberOfBytesTransferred);
    UNREFERENCED_PARAMETER(Instance);
    UNREFERENCED_PARAMETER(pContext);

    PHTTP_IO_CONTEXT pIoContext = CONTAINING_RECORD(pOverlapped, 
                                                    HTTP_IO_CONTEXT, 
                                                    Overlapped);

    pServerContext = pIoContext->pServerContext;

    pIoContext->pfCompletionFunction(pIoContext, Io, IoResult);
}

//
// Routine Description:
// 
//     Initializes the Url and server directory using command line parameters,
//     accesses the HTTP Server API driver, creates a server session, creates 
//     a Url Group under the specified server session, adds the specified Url to 
//     the Url Group.
//
//
// Arguments:
//     pwszUrlPathToListenFor - URL path the user wants this sample to listen
//                              on.
//
//     pwszRootDirectory - Root directory on this host to which we will map
//                         incoming URLs
//
//     pServerContext - The server we are associated with.
//
// Return Value:
// 
//     TRUE, if http server was initialized successfully, 
//     otherwise returns FALSE.
// 

BOOL InitializeHttpServer(
                          PWCHAR pwszUrlPathToListenFor,
                          PWCHAR pwszRootDirectory,
                          PSERVER_CONTEXT pServerContext
                          )
{
    ULONG ulResult;
    HRESULT hResult;

    hResult = StringCbCopyW(
                pServerContext->wszRootDirectory, 
                MAX_STR_SIZE, 
                pwszRootDirectory);

    if (FAILED(hResult))
    {
        fprintf(stderr, "Invalid command line arguments. Application stopped.\n");
        return FALSE;
    }

    ulResult = HttpInitialize(
        g_HttpApiVersion, 
        HTTP_INITIALIZE_SERVER, 
        NULL);

    if (ulResult != NO_ERROR)
    {
        fprintf(stderr, "HttpInitialized failed\n");
        return FALSE;
    }

    pServerContext->bHttpInit = TRUE;

    ulResult = HttpCreateServerSession(
        g_HttpApiVersion, 
        &(pServerContext->sessionId), 
        0);
    
    if (ulResult != NO_ERROR)
    {
        fprintf(stderr, "HttpCreateServerSession failed\n");
        return FALSE;
    }

    ulResult = HttpCreateUrlGroup(
        pServerContext->sessionId, 
        &(pServerContext->urlGroupId), 
        0);
    
    if (ulResult != NO_ERROR)
    {
        fprintf(stderr, "HttpCreateUrlGroup failed\n");
        return FALSE;
    }

    ulResult = HttpAddUrlToUrlGroup(
        pServerContext->urlGroupId, 
        pwszUrlPathToListenFor, 
        (HTTP_URL_CONTEXT) NULL, 
        0);
    
    if (ulResult != NO_ERROR)
    {
        fwprintf(stderr, L"HttpAddUrlToUrlGroup failed with code 0x%x for url %s\n",
            ulResult, pwszUrlPathToListenFor);
        return FALSE;
    }

    return TRUE;
}

//
// Routine Description:
// 
//      Creates the stop server event. We will set it when all active IO 
//      operations on the API have completed, allowing us to cleanup, creates a 
//      new request queue, sets a new property on the specified Url group,
//      creates a new I/O completion.
//
//
// Arguments:
//    
//     pServerContext - The server we are associated with.
//
// Return Value:
// 
//     TRUE, if http server was initialized successfully, 
//     otherwise returns FALSE.
//

BOOL InitializeServerIo(
                        PSERVER_CONTEXT pServerContext
                        )
{
    ULONG Result;
    HTTP_BINDING_INFO HttpBindingInfo = {0};

    Result = HttpCreateRequestQueue(
        g_HttpApiVersion, 
        L"Test_Http_Server_HTTPAPI_V2",
        NULL, 
        0, 
        &(pServerContext->hRequestQueue));

    if (Result != NO_ERROR)
    {
        fprintf(stderr, "HttpCreateRequestQueue failed\n");
        return FALSE;
    }

    HttpBindingInfo.Flags.Present       = 1;
    HttpBindingInfo.RequestQueueHandle  = pServerContext->hRequestQueue;

    Result = HttpSetUrlGroupProperty(
        pServerContext->urlGroupId,
        HttpServerBindingProperty,
        &HttpBindingInfo,
        sizeof(HttpBindingInfo));
    
    if (Result != NO_ERROR)
    {
        fprintf(stderr, "HttpSetUrlGroupProperty(...HttpServerBindingProperty...) failed\n");
        return FALSE;
    }

    pServerContext->Io = CreateThreadpoolIo(
        pServerContext->hRequestQueue,
        IoCompletionCallback,
        NULL,
        NULL);

    if (pServerContext->Io == NULL)
    {
        fprintf(stderr, "Creating a new I/O completion object failed\n");
        return FALSE;
    }

    return TRUE;
}

//
// Routine Description:
// 
//     Calculates the number of processors and post a proportional number of
//     receive requests.
//
// Arguments:
//    
//     pServerContext - The server we are associated with.
//
// Return Value:
// 
//    TRUE, if http server was initialized successfully,
//    otherwise returns FALSE.
//

BOOL StartServer(
                 PSERVER_CONTEXT pServerContext
                 )
{
    DWORD_PTR dwProcessAffinityMask, dwSystemAffinityMask;
    WORD wRequestsCounter;
    BOOL bGetProcessAffinityMaskSucceed;

    bGetProcessAffinityMaskSucceed = GetProcessAffinityMask(
                                        GetCurrentProcess(), 
                                        &dwProcessAffinityMask, 
                                        &dwSystemAffinityMask);

    if(bGetProcessAffinityMaskSucceed)
    {
        for (wRequestsCounter = 0; dwProcessAffinityMask; dwProcessAffinityMask >>= 1)
        {
            if (dwProcessAffinityMask & 0x1) wRequestsCounter++;
        }
        
        wRequestsCounter = REQUESTS_PER_PROCESSOR * wRequestsCounter;
    }
    else
    {
        fprintf(stderr, 
                "We could not calculate the number of processor's, "
                "the server will continue with the default number = %d\n", 
                OUTSTANDING_REQUESTS);

        wRequestsCounter = OUTSTANDING_REQUESTS;
    }

    for (; wRequestsCounter > 0; --wRequestsCounter)
    {
        PHTTP_IO_REQUEST pIoRequest;
        ULONG Result;

        pIoRequest = AllocateHttpIoRequest(pServerContext);

        if (pIoRequest == NULL)
        {
            fprintf(stderr, "AllocateHttpIoRequest failed for context %d\n", wRequestsCounter);
            return FALSE;
        }

        StartThreadpoolIo(pServerContext->Io);

        Result = HttpReceiveHttpRequest(
            pServerContext->hRequestQueue, 
            HTTP_NULL_ID, 
            HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY, 
            pIoRequest->pHttpRequest,
            sizeof(pIoRequest->RequestBuffer),
            NULL,
            &pIoRequest->ioContext.Overlapped
            );

        if (Result != ERROR_IO_PENDING && Result != NO_ERROR)
        {
            CancelThreadpoolIo(pServerContext->Io);

            if (Result == ERROR_MORE_DATA)
            {
                ProcessReceiveAndPostResponse(pIoRequest, pServerContext->Io, ERROR_MORE_DATA);
            }

            CleanupHttpIoRequest(pIoRequest);

            fprintf(stderr, "HttpReceiveHttpRequest failed, error 0x%lx\n", Result);

            return FALSE;
        }
    }
    
    return TRUE;
}

//
// Routine Description:
// 
//     Stops queuing requests for the specified request queue process, 
//     waits for the pended requests to be completed, 
//     waits for I/O completion callbacks to complete. 
//
// Arguments:
//    
//     pServerContext - The server we are associated with.
//
// Return Value:
// 
//     N/A
// 

VOID StopServer(
                PSERVER_CONTEXT pServerContext
                )
{
    if (pServerContext->hRequestQueue != NULL)
    {
        pServerContext->bStopServer = TRUE;
        
        HttpShutdownRequestQueue(pServerContext->hRequestQueue);
    }

    if (pServerContext->Io != NULL)
    {
        //
        // This call will block until all IO complete their callbacks.
        WaitForThreadpoolIoCallbacks(pServerContext->Io, FALSE);
    }
}

//
// Routine Description:
// 
//      Closes the handle to the specified request queue, releases the specified 
//      I/O completion object, deletes the stop server event.
//
//
// Arguments:
//    
//     pServerContext - The server we are associated with.
//
// Return Value:
// 
//     N/A
// 

VOID UninitializeServerIo(
                          PSERVER_CONTEXT pServerContext
                          )
{   
    if (pServerContext->hRequestQueue != NULL)
    {
        HttpCloseRequestQueue(pServerContext->hRequestQueue);
        pServerContext->hRequestQueue = NULL;
    }

    if (pServerContext->Io != NULL)
    {
        CloseThreadpoolIo(pServerContext->Io);
        pServerContext->Io = NULL;
    }
}


//
// Routine Description:
//
//     Closes the Url Group, deletes the server session 
//     cleans up resources used by the HTTP Server API.
//
//
// Arguments:
//    
//     pServerContext - The server we are associated with.
//
// Return Value:
// 
//     N/A
// 

VOID UninitializeHttpServer(
                            PSERVER_CONTEXT pServerContext
                            )
{
    if (pServerContext->urlGroupId != 0)
    {
        HttpCloseUrlGroup(pServerContext->urlGroupId);
        pServerContext->urlGroupId = 0;
    }
    
    if (pServerContext->sessionId != 0)
    {
        HttpCloseServerSession(pServerContext->sessionId);
        pServerContext->sessionId = 0;
    }
    
    if (pServerContext->bHttpInit == TRUE)
    {
        HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
        pServerContext->bHttpInit = FALSE;
    }
}

//
// Routine Description:
// 
//     Step by step: 
//          - checks the number of command line parameters,
//          - initializes the http server, if failed uninitializes the http server, 
//          - initializes http server Io completion object, if failed 
//            uninitializes it and uninitializes the http server,
//          - starts http server, if failed stops http server, uninitializes,
//            Io completion object object and uninitializes the http server.
//
//     Cleans-up upon user input. The clean up process consists of:
//
//          - uninitializes the http server,
//          - uninitializes Io completion object,
//          - uninitializes the http server. 
//
// Arguments:
//     argc - Contains the count of arguments that follow in argv. 
// 
//     argv - An array of null-terminated strings representing command-line 
//            Expected:
//              argv[1] - is the Url base this sample will listen for.
//              argv[2] - is the local directory to map incoming requested Url
//                        to locally.
//
// Return Value:
// 
//     Exit code.
// 

DWORD wmain(
            DWORD argc, 
            WCHAR **argv
            )
{
    SERVER_CONTEXT ServerContext;

    ZeroMemory(&ServerContext, sizeof(SERVER_CONTEXT));

    if (argc != 3)
        return FALSE;

    if (wcslen(argv[1]) > MAX_STR_SIZE ||
        wcslen(argv[2]) > MAX_STR_SIZE)
        return FALSE;

    if (!InitializeHttpServer(argv[1], argv[2], &ServerContext))
        goto CleanServer;

    if (!InitializeServerIo(&ServerContext))
        goto CleanIo;

    if (!StartServer(&ServerContext))
        goto StopServer;

    printf("HTTP server is running.\n");
    printf("Press any key to stop.\n");

    // Waiting for the user command.

    _getch();

StopServer:
    StopServer(&ServerContext);

CleanIo:
    UninitializeServerIo(&ServerContext);

CleanServer:
    UninitializeHttpServer(&ServerContext);

    printf("Done.\n");

    return 0;
}
