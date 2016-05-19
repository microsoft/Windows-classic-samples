 /*++
 Copyright (c) 2002 - 2002 Microsoft Corporation.  All Rights Reserved.

 THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 THIS CODE IS NOT SUPPORTED BY MICROSOFT.

--*/

#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4127)   // condition expression is constant

#define _WINSOCKAPI_
#include    <windows.h>
#include    <stdio.h>
#include    <strsafe.h>
#include    "http.h"


//
// Macros.
//
#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )                    \
    do                                                                      \
    {                                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );                           \
        (resp)->StatusCode = (status);                                      \
        (resp)->pReason = (reason);                                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);                     \
    } while (FALSE)



#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)                      \
    do                                                                      \
    {                                                                       \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue = (RawValue); \
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength =        \
            (USHORT) strlen(RawValue);                                      \
    } while(FALSE)

#define ALLOC_MEM(cb) HeapAlloc(GetProcessHeap(), 0, (cb))
#define FREE_MEM(ptr) HeapFree(GetProcessHeap(), 0, (ptr))

//
// Prototypes.
//
DWORD
DoReceiveRequests(
    HANDLE hReqQueue
    );

DWORD
SendHttpResponse(
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest,
    IN USHORT StatusCode,
    __in IN PSTR pReason,
    __in_opt IN PSTR pEntity
    );

DWORD
SendHttpPostResponse(
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest
    );

/***************************************************************************++

Routine Description:
    main routine.

Arguments:
    argc - # of command line arguments.
    argv - Arguments.

Return Value:
    Success/Failure.

--***************************************************************************/
int
__cdecl
wmain(
    int argc,
    __in_ecount(argc) wchar_t * argv[]
    )
{
    ULONG           retCode;
    int             i;
    HANDLE          hReqQueue      = NULL;
    int             UrlAdded       = 0;
    HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;

    if (argc < 2)
    {
        wprintf(L"%ws: <Url1> [Url2] ... \n", argv[0]);
        return -1;
    }

    //
    // Initialize HTTP APIs.
    //
    retCode = HttpInitialize(
                HttpApiVersion,
                HTTP_INITIALIZE_SERVER,    // Flags
                NULL                       // Reserved
                );

    if (retCode != NO_ERROR)
    {
        wprintf(L"HttpInitialize failed with %lu \n", retCode);
        return retCode;
    }

    //
    // Create a Request Queue Handle
    //
    retCode = HttpCreateHttpHandle(
                &hReqQueue,        // Req Queue
                0                  // Reserved
                );

    if (retCode != NO_ERROR)
    {
        wprintf(L"HttpCreateHttpHandle failed with %lu \n", retCode);
        goto CleanUp;
    }

    //
    // The command line arguments represent URIs that we want to listen on.
    // We will call HttpAddUrl for each of these URIs.
    //
    // The URI is a fully qualified URI and MUST include the terminating '/'
    //
    for (i = 1; i < argc; i++)
    {
        wprintf(
          L"we are listening for requests on the following url: %s\n",
          argv[i]);

        retCode = HttpAddUrl(
                    hReqQueue,    // Req Queue
                    argv[i],      // Fully qualified URL
                    NULL          // Reserved
                    );

        if (retCode != NO_ERROR)
        {
            wprintf(L"HttpAddUrl failed with %lu \n", retCode);
            goto CleanUp;
        }
        else
        {
            //
            // Keep track of the URLs that we've currently added.
            //
            UrlAdded ++;
        }
    }

    // Loop while receiving requests
    DoReceiveRequests(hReqQueue);

CleanUp:

    //
    // Call HttpRemoveUrl for all the URLs that we added.
    //
    for(i=1; i<=UrlAdded; i++)
    {
        HttpRemoveUrl(
              hReqQueue,     // Req Queue
              argv[i]        // Fully qualified URL
              );
    }

    //
    // Close the Request Queue handle.
    //
    if(hReqQueue)
    {
        CloseHandle(hReqQueue);
    }

    //
    // Call HttpTerminate.
    //
    HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);

    return retCode;
}

/***************************************************************************++

Routine Description:
    The routine to receive a request. This routine calls the corresponding
    routine to deal with the response.

Arguments:
    hReqQueue - Handle to the request queue.

Return Value:
    Success/Failure.

--***************************************************************************/
DWORD
DoReceiveRequests(
    IN HANDLE hReqQueue
    )
{
    ULONG              result;
    HTTP_REQUEST_ID    requestId;
    DWORD              bytesRead;
    PHTTP_REQUEST      pRequest;
    PCHAR              pRequestBuffer;
    ULONG              RequestBufferLength;

    //
    // Allocate a 2K buffer. Should be good for most requests, we'll grow
    // this if required. We also need space for a HTTP_REQUEST structure.
    //
    RequestBufferLength = sizeof(HTTP_REQUEST) + 2048;
    pRequestBuffer      = ALLOC_MEM( RequestBufferLength );

    if (pRequestBuffer == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRequest = (PHTTP_REQUEST)pRequestBuffer;

    //
    // Wait for a new request -- This is indicated by a NULL request ID.
    //

    HTTP_SET_NULL_ID( &requestId );

    for(;;)
    {
        RtlZeroMemory(pRequest, RequestBufferLength);

        result = HttpReceiveHttpRequest(
                    hReqQueue,          // Req Queue
                    requestId,          // Req ID
                    0,                  // Flags
                    pRequest,           // HTTP request buffer
                    RequestBufferLength,// req buffer length
                    &bytesRead,         // bytes received
                    NULL                // LPOVERLAPPED
                    );

        if(NO_ERROR == result)
        {
            //
            // Worked!
            //
            switch(pRequest->Verb)
            {
                case HttpVerbGET:
                    wprintf(L"Got a GET request for %ws \n",
                            pRequest->CookedUrl.pFullUrl);

                    result = SendHttpResponse(
                                hReqQueue,
                                pRequest,
                                200,
                                "OK",
                                "Hey! You hit the server \r\n"
                                );
                    break;

                case HttpVerbPOST:

                    wprintf(L"Got a POST request for %ws \n",
                            pRequest->CookedUrl.pFullUrl);

                    result = SendHttpPostResponse(hReqQueue, pRequest);
                    break;

                default:
                    wprintf(L"Got a unknown request for %ws \n",
                            pRequest->CookedUrl.pFullUrl);

                    result = SendHttpResponse(
                                hReqQueue,
                                pRequest,
                                503,
                                "Not Implemented",
                                NULL
                                );
                    break;
            }

            if(result != NO_ERROR)
            {
                break;
            }

            //
            // Reset the Request ID so that we pick up the next request.
            //
            HTTP_SET_NULL_ID( &requestId );
        }
        else if(result == ERROR_MORE_DATA)
        {
            //
            // The input buffer was too small to hold the request headers
            // We have to allocate more buffer & call the API again.
            //
            // When we call the API again, we want to pick up the request
            // that just failed. This is done by passing a RequestID.
            //
            // This RequestID is picked from the old buffer.
            //
            requestId = pRequest->RequestId;

            //
            // Free the old buffer and allocate a new one.
            //
            RequestBufferLength = bytesRead;
            FREE_MEM( pRequestBuffer );
            pRequestBuffer = ALLOC_MEM( RequestBufferLength );

            if (pRequestBuffer == NULL)
            {
                result = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            pRequest = (PHTTP_REQUEST)pRequestBuffer;

        }
        else if(ERROR_CONNECTION_INVALID == result &&
                !HTTP_IS_NULL_ID(&requestId))
        {
            // The TCP connection got torn down by the peer when we were
            // trying to pick up a request with more buffer. We'll just move
            // onto the next request.

            HTTP_SET_NULL_ID( &requestId );
        }
        else
        {
            break;
        }

    } // for(;;)

    if(pRequestBuffer)
    {
        FREE_MEM( pRequestBuffer );
    }

    return result;
}

/***************************************************************************++

Routine Description:
    The routine sends a HTTP response.

Arguments:
    hReqQueue     - Handle to the request queue.
    pRequest      - The parsed HTTP request.
    StatusCode    - Response Status Code.
    pReason       - Response reason phrase.
    pEntityString - Response entity body.

Return Value:
    Success/Failure.

--***************************************************************************/
DWORD
SendHttpResponse(
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest,
    IN USHORT StatusCode,
    __in IN PSTR pReason,
    __in_opt IN PSTR pEntityString
    )
{
    HTTP_RESPONSE   response;
    HTTP_DATA_CHUNK dataChunk;
    DWORD           result;
    DWORD           bytesSent;

    //
    // Initialize the HTTP response structure.
    //
    INITIALIZE_HTTP_RESPONSE(&response, StatusCode, pReason);

    //
    // Add a known header.
    //
    ADD_KNOWN_HEADER(response, HttpHeaderContentType, "text/html");

    if(pEntityString)
    {
        //
        // Add an entity chunk
        //
        dataChunk.DataChunkType           = HttpDataChunkFromMemory;
        dataChunk.FromMemory.pBuffer      = pEntityString;
        dataChunk.FromMemory.BufferLength = (ULONG) strlen(pEntityString);

        response.EntityChunkCount         = 1;
        response.pEntityChunks            = &dataChunk;
    }

    //
    // Since we are sending all the entity body in one call, we don't have
    // to specify the Content-Length.
    //

    result = HttpSendHttpResponse(
                    hReqQueue,           // ReqQueueHandle
                    pRequest->RequestId, // Request ID
                    0,                   // Flags
                    &response,           // HTTP response
                    NULL,                // pReserved1
                    &bytesSent,          // bytes sent   (OPTIONAL)
                    NULL,                // pReserved2   (must be NULL)
                    0,                   // Reserved3    (must be 0)
                    NULL,                // LPOVERLAPPED (OPTIONAL)
                    NULL                 // pReserved4   (must be NULL)
                    );

    if(result != NO_ERROR)
    {
        wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
    }

    return result;
}

/***************************************************************************++

Routine Description:
    The routine sends a HTTP response after reading the entity body.

Arguments:
    hReqQueue     - Handle to the request queue.
    pRequest      - The parsed HTTP request.

Return Value:
    Success/Failure.

--***************************************************************************/
DWORD
SendHttpPostResponse(
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest
    )
{
    HTTP_RESPONSE   response;
    DWORD           result;
    DWORD           bytesSent;
    PUCHAR          pEntityBuffer;
    ULONG           EntityBufferLength;
    ULONG           BytesRead;
    ULONG           TempFileBytesWritten;
    HANDLE          hTempFile;
    TCHAR           szTempName[MAX_PATH + 1];
#define MAX_ULONG_STR ((ULONG) sizeof("4294967295"))
    CHAR            szContentLength[MAX_ULONG_STR];
    HTTP_DATA_CHUNK dataChunk;
    ULONG           TotalBytesRead = 0;

    BytesRead  = 0;
    hTempFile  = INVALID_HANDLE_VALUE;

    //
    // Allocate some space for an entity buffer. We'll grow this on demand.
    //
    EntityBufferLength = 2048;
    pEntityBuffer      = ALLOC_MEM( EntityBufferLength );

    if (pEntityBuffer == NULL)
    {
        result = ERROR_NOT_ENOUGH_MEMORY;
        wprintf(L"Insufficient resources \n");
        goto Done;
    }

    //
    // Initialize the HTTP response structure.
    //
    INITIALIZE_HTTP_RESPONSE(&response, 200, "OK");

    //
    // For POST, we'll echo back the entity that we got from the client.
    //
    // NOTE: If we had passed the HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY
    //       flag with HttpReceiveHttpRequest(), the entity would have
    //       been a part of HTTP_REQUEST (using the pEntityChunks field).
    //       Since we have not passed that flag, we can be assured that
    //       there are no entity bodies in HTTP_REQUEST.
    //

    if(pRequest->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS)
    {
        // The entity body is send over multiple calls. Let's collect all
        // of these in a file & send it back. We'll create a temp file
        //

        if(GetTempFileName(
                L".",
                L"New",
                0,
                szTempName
                ) == 0)
        {
            result = GetLastError();
            wprintf(L"GetTempFileName failed with %lu \n", result);
            goto Done;
        }

        hTempFile = CreateFile(
                        szTempName,
                        GENERIC_READ | GENERIC_WRITE,
                        0,                             // don't share.
                        NULL,                          // no security descriptor
                        CREATE_ALWAYS,                 // overrwrite existing
                        FILE_ATTRIBUTE_NORMAL,         // normal file.
                        NULL
                        );

        if(hTempFile == INVALID_HANDLE_VALUE)
        {
            result = GetLastError();
            wprintf(L"Could not create temporary file. Error %lu \n", result);
            goto Done;
        }

        do
        {
            //
            // Read the entity chunk from the request.
            //
            BytesRead = 0;
            result = HttpReceiveRequestEntityBody(
                        hReqQueue,
                        pRequest->RequestId,
                        0,
                        pEntityBuffer,
                        EntityBufferLength,
                        &BytesRead,
                        NULL
                        );

            switch(result)
            {
                case NO_ERROR:

                    if(BytesRead != 0)
                    {
                        TotalBytesRead += BytesRead;
                        WriteFile(
                                hTempFile,
                                pEntityBuffer,
                                BytesRead,
                                &TempFileBytesWritten,
                                NULL
                                );
                    }
                    break;

                case ERROR_HANDLE_EOF:

                    //
                    // We have read the last request entity body. We can send
                    // back a response.
                    //
                    // To illustrate entity sends via
                    // HttpSendResponseEntityBody, we will send the response
                    // over multiple calls. This is achieved by passing the
                    // HTTP_SEND_RESPONSE_FLAG_MORE_DATA flag.

                    if(BytesRead != 0)
                    {
                        TotalBytesRead += BytesRead;
                        WriteFile(
                                hTempFile,
                                pEntityBuffer,
                                BytesRead,
                                &TempFileBytesWritten,
                                NULL
                                );
                    }

                    //
                    // Since we are sending the response over multiple API
                    // calls, we have to add a content-length.
                    //
                    // Alternatively, we could have sent using chunked transfer
                    // encoding, by passing "Transfer-Encoding: Chunked".
                    //

                    // NOTE: Since we are accumulating the TotalBytesRead in
                    //       a ULONG, this will not work for entity bodies that
                    //       are larger than 4 GB. For supporting large entity
                    //       bodies, we would have to use a ULONGLONG.
                    //


                    StringCchPrintfA(
                            szContentLength,
                            sizeof(szContentLength),
                            "%lu",
                            TotalBytesRead
                            );

                    ADD_KNOWN_HEADER(
                            response,
                            HttpHeaderContentLength,
                            szContentLength
                            );

                    result =
                        HttpSendHttpResponse(
                               hReqQueue,           // ReqQueueHandle
                               pRequest->RequestId, // Request ID
                               HTTP_SEND_RESPONSE_FLAG_MORE_DATA,
                               &response,           // HTTP response
                               NULL,                // pReserved1
                               &bytesSent,          // bytes sent (optional)
                               NULL,                // pReserved2
                               0,                   // Reserved3
                               NULL,                // LPOVERLAPPED
                               NULL                 // pReserved4
                               );

                    if(result != NO_ERROR)
                    {
                        wprintf(L"HttpSendHttpResponse failed with %lu \n",
                                result);
                        goto Done;
                    }

                    //
                    // Send entity body from a file handle.
                    //
                    dataChunk.DataChunkType =
                        HttpDataChunkFromFileHandle;

                    dataChunk.FromFileHandle.
                        ByteRange.StartingOffset.QuadPart = 0;

                    dataChunk.FromFileHandle.
                        ByteRange.Length.QuadPart = HTTP_BYTE_RANGE_TO_EOF;

                    dataChunk.FromFileHandle.FileHandle = hTempFile;

                    result = HttpSendResponseEntityBody(
                                hReqQueue,
                                pRequest->RequestId,
                                0,                    // This is the last send.
                                1,                    // Entity Chunk Count.
                                &dataChunk,
                                NULL,
                                NULL,
                                0,
                                NULL,
                                NULL
                                );

                    if(result != NO_ERROR)
                    {
                        wprintf(
                           L"HttpSendResponseEntityBody failed with %lu \n",
                           result
                           );
                    }

                    goto Done;

                    break;


                default:
                    wprintf(L"HttpReceiveRequestEntityBody failed with %lu \n",
                            result);
                    goto Done;
            }

        } while(TRUE);
    }
    else
    {
        // This request does not have any entity body.
        //

        result = HttpSendHttpResponse(
                   hReqQueue,           // ReqQueueHandle
                   pRequest->RequestId, // Request ID
                   0,
                   &response,           // HTTP response
                   NULL,                // pReserved1
                   &bytesSent,          // bytes sent (optional)
                   NULL,                // pReserved2
                   0,                   // Reserved3
                   NULL,                // LPOVERLAPPED
                   NULL                 // pReserved4
                   );
        if(result != NO_ERROR)
        {
            wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
        }
    }

Done:

    if(pEntityBuffer)
    {
        FREE_MEM(pEntityBuffer);
    }

    if(INVALID_HANDLE_VALUE != hTempFile)
    {
        CloseHandle(hTempFile);
        DeleteFile(szTempName);
    }

    return result;
}
