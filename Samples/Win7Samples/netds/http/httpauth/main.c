/*++
 Copyright (c) 2002 - 2002 Microsoft Corporation.  All Rights Reserved.

 THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 THIS CODE IS NOT SUPPORTED BY MICROSOFT.

 This sample code demonstrates the use of http.sys Authentication feaure.

--*/

#define SECURITY_WIN32
#include <http.h>
#include <sspi.h>
#include <strsafe.h>
#define NUM_SCHEMES 2
#define MAX_USERNAME_LENGTH 100

#pragma warning(disable:4127)   // condition expression is constant


//
// Macros.
//
#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )                    \
    {                                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );                           \
        (resp)->StatusCode = (status);                                      \
        (resp)->pReason = (reason);                                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);                     \
    } 



#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)                      \
    {                                                                       \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue = (RawValue); \
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength =        \
            (USHORT) strlen(RawValue);                                      \
    } 



#define ALLOC_MEM(cb) HeapAlloc(GetProcessHeap(), 0, (cb))
#define FREE_MEM(ptr) HeapFree(GetProcessHeap(), 0, (ptr))



PCSTR
MapSecurityErrorToString(
    IN SECURITY_STATUS Error
    )
{
    switch(Error) {

    case SEC_E_OK                           : return "SEC_E_OK";
    case SEC_E_INSUFFICIENT_MEMORY          : return "SEC_E_INSUFFICIENT_MEMORY";
    case SEC_E_INVALID_HANDLE               : return "SEC_E_INVALID_HANDLE";
    case SEC_E_UNSUPPORTED_FUNCTION         : return "SEC_E_UNSUPPORTED_FUNCTION";
    case SEC_E_TARGET_UNKNOWN               : return "SEC_E_TARGET_UNKNOWN";
    case SEC_E_INTERNAL_ERROR               : return "SEC_E_INTERNAL_ERROR";
    case SEC_E_SECPKG_NOT_FOUND             : return "SEC_E_SECPKG_NOT_FOUND";
    case SEC_E_NOT_OWNER                    : return "SEC_E_NOT_OWNER";
    case SEC_E_CANNOT_INSTALL               : return "SEC_E_CANNOT_INSTALL";
    case SEC_E_INVALID_TOKEN                : return "SEC_E_INVALID_TOKEN";
    case SEC_E_CANNOT_PACK                  : return "SEC_E_CANNOT_PACK";
    case SEC_E_QOP_NOT_SUPPORTED            : return "SEC_E_QOP_NOT_SUPPORTED";
    case SEC_E_NO_IMPERSONATION             : return "SEC_E_NO_IMPERSONATION";
    case SEC_E_LOGON_DENIED                 : return "SEC_E_LOGON_DENIED";
    case SEC_E_UNKNOWN_CREDENTIALS          : return "SEC_E_UNKNOWN_CREDENTIALS";
    case SEC_E_NO_CREDENTIALS               : return "SEC_E_NO_CREDENTIALS";
    case SEC_E_MESSAGE_ALTERED              : return "SEC_E_MESSAGE_ALTERED";
    case SEC_E_OUT_OF_SEQUENCE              : return "SEC_E_OUT_OF_SEQUENCE";
    case SEC_E_NO_AUTHENTICATING_AUTHORITY  : return "SEC_E_NO_AUTHENTICATING_AUTHORITY";
    case SEC_E_BAD_PKGID                    : return "SEC_E_BAD_PKGID";
    case SEC_E_TIME_SKEW                    : return "SEC_E_TIME_SKEW";

    default                                 : return "SEC_E_ERROR";
    }
}

//
// Prototypes.
//
DWORD
DoReceiveRequests(
    IN HANDLE hReqQueue
    );

DWORD
SendHttpResponse(
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest,
    IN  ULONG Flags,
    IN USHORT StatusCode,
    __in IN PSTR pReason,
    __in_opt IN PSTR pEntityString
    );

DWORD
Send401HttpResponse(
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest,
    __in_opt IN PSTR pEntityString
    );

DWORD
SendHttpPostResponse(
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest
    );

HTTP_REQUEST_AUTH_INFO *
GetAuthInfo(
    IN PHTTP_REQUEST pRequest
    );

DWORD
HandleAuthRequest(
    IN  PHTTP_REQUEST_AUTH_INFO pAuthInfo,
    IN HANDLE hReqQueue,
    PHTTP_REQUEST pRequest
    );

DWORD
UseSerializedContext(
    IN  PHTTP_REQUEST_AUTH_INFO pAuthInfo,
    IN OUT PCtxtHandle phContext
    );

DWORD
UseAccessToken(
    IN  PHTTP_REQUEST_AUTH_INFO pAuthInfo
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
    int i;
    HANDLE          hReqQueue      = NULL;
    HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_2;
    HTTP_SERVER_SESSION_ID ssID = HTTP_NULL_ID;
    HTTP_URL_GROUP_ID urlGroupId = HTTP_NULL_ID;
    HTTP_BINDING_INFO BindingProperty;
    HTTP_SERVER_AUTHENTICATION_INFO Config;
    ZeroMemory(&Config, sizeof(HTTP_SERVER_AUTHENTICATION_INFO));



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
    // Create a server session handle
    //

    retCode = HttpCreateServerSession(HttpApiVersion,
                                      &ssID,
                                      0);


    if (retCode != NO_ERROR)
    {
        wprintf(L"HttpCreateServerSession failed with %lu \n", retCode);
        ssID = HTTP_NULL_ID;
        goto CleanUp;
    }


    //
    // Create UrlGroup handle
    //

    retCode = HttpCreateUrlGroup(ssID,  
                                 &urlGroupId, 
                                 0);


    if (retCode != NO_ERROR)
    {
        wprintf(L"HttpCreateUrlGroup failed with %lu \n", retCode);
        urlGroupId = HTTP_NULL_ID;
        goto CleanUp;
    }
    
    //
    // Create a request queue handle
    //

    retCode = HttpCreateRequestQueue(HttpApiVersion,
                                     L"MyQueue",
                                     NULL,
                                     0,
                                     &hReqQueue);


    if (retCode != NO_ERROR)
    {
        wprintf(L"HttpCreateRequestQueue failed with %lu \n", retCode);
        hReqQueue      = NULL;
        goto CleanUp;
    }



    BindingProperty.Flags.Present = 1;// Specifies that the property is present on UrlGroup
    BindingProperty.RequestQueueHandle = hReqQueue;

    
    //
    // Bind the request queue to UrlGroup
    //

    retCode = HttpSetUrlGroupProperty(urlGroupId,
                                      HttpServerBindingProperty,
                                      &BindingProperty,
                                      sizeof(BindingProperty));

    if (retCode != NO_ERROR)
    {
        wprintf(L"HttpSetUrlGroupProperty failed with %lu \n", retCode);
        goto CleanUp;
    }


    //
    // Set Auth property on UrlGroup
    //


    Config.Flags.Present = 1;
    Config.AuthSchemes = HTTP_AUTH_ENABLE_NTLM | HTTP_AUTH_ENABLE_NEGOTIATE,
    Config.ReceiveMutualAuth = FALSE;
    Config.ReceiveContextHandle = TRUE;
    Config.DisableNTLMCredentialCaching = TRUE;


    retCode = HttpSetUrlGroupProperty(urlGroupId,
                                      HttpServerAuthenticationProperty,
                                      &Config,
                                      sizeof(HTTP_SERVER_AUTHENTICATION_INFO));

    if (retCode != NO_ERROR)
    {
        wprintf(L"HttpSetUrlGroupProperty failed with %lu \n", retCode);
        goto CleanUp;
    }


    //
    // Add the URLs on URL Group
    // The command line arguments represent URIs that we want to listen on.
    // We will call HttpAddUrlToUrlGroup for each of these URIs.
    //
    // The URI is a fully qualified URI and MUST include the terminating '/'
    //
    for (i = 1; i < argc; i++)
    {
        wprintf(
          L"we are listening for requests on the following url: %s\n",
          argv[i]);


        retCode = HttpAddUrlToUrlGroup(urlGroupId,
                                       argv[i],
                                       0,
                                       0);


        if (retCode != NO_ERROR)
        {
            wprintf(L"HttpAddUrl failed with %lu \n", retCode);
            goto CleanUp;
        }
    }

    // Loop while receiving requests
    DoReceiveRequests(hReqQueue);

CleanUp:

    //
    // Call HttpRemoveUrl for all the URLs that we added.
    // HTTP_URL_FLAG_REMOVE_ALL flag allows us to remove
    // all the URLs registered on URL Group at once
    //
    if (!HTTP_IS_NULL_ID(&urlGroupId)) 
    {

        retCode = HttpRemoveUrlFromUrlGroup(urlGroupId,
                                            NULL,
                                            HTTP_URL_FLAG_REMOVE_ALL);

       if (retCode != NO_ERROR)
       {
           wprintf(L"HttpRemoveUrl failed with %lu \n", retCode);
       }

    }

    //
    // Close the Url Group
    //

    if (!HTTP_IS_NULL_ID(&urlGroupId)) 
    {
        retCode = HttpCloseUrlGroup(urlGroupId);

       if (retCode != NO_ERROR)
       {
           wprintf(L"HttpCloseUrlGroup failed with %lu \n", retCode);
       }

    }


    //
    // Close the serversession
    //
    
    if (!HTTP_IS_NULL_ID(&ssID)) 
    {
        retCode = HttpCloseServerSession(ssID);

       if (retCode != NO_ERROR)
       {
           wprintf(L"HttpCloseServerSession failed with %lu \n", retCode);
       }

    }

    //
    // Close the Request Queue handle.
    //

    if(hReqQueue)
    {
        retCode = HttpCloseRequestQueue(hReqQueue);

       if (retCode != NO_ERROR)
       {
           wprintf(L"HttpCloseRequestQueue failed with %lu \n", retCode);
       }
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
            //Deal with Authentication here
            //

            HTTP_REQUEST_AUTH_INFO *pAuthInfo = NULL;

            pAuthInfo = GetAuthInfo(pRequest);

            if (NULL == pAuthInfo)
            {
                wprintf(L"GetAuthInfo failed!?\n");
            }

            //
            //Handle Authentication
            //
            
            result = HandleAuthRequest(
                            pAuthInfo,
                            hReqQueue,
                            pRequest);

            if (NO_ERROR != result)
            {
                wprintf(L"HandleAuthRequest failed %d\n", result);
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
                wprintf(L"Send 503 Service Unavailable\n");

                SendHttpResponse(
                                hReqQueue,
                                pRequest,
                                HTTP_SEND_RESPONSE_FLAG_DISCONNECT,
                                503,
                                "Unavailable",
                                "Resource Unavailable\n"
                                );

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
    IN  ULONG Flags,
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
                    Flags,                   // Flags
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


BOOL AddMultipleKnownHeader(
    IN PHTTP_RESPONSE pResponse,
    IN HTTP_HEADER_ID HeaderId,
    IN PHTTP_KNOWN_HEADER pkHeaders,
    IN USHORT numEntries,
    IN ULONG flags
    )
{
    PHTTP_MULTIPLE_KNOWN_HEADERS pHeader = NULL;
    PHTTP_RESPONSE_INFO pResponseInfo = NULL;
    
    pHeader = ALLOC_MEM(sizeof(HTTP_MULTIPLE_KNOWN_HEADERS) );
    if(!pHeader)
        return FALSE;
    
    ZeroMemory(pHeader, sizeof(HTTP_MULTIPLE_KNOWN_HEADERS));
    pHeader->HeaderId = HeaderId; 
    pHeader->KnownHeaderCount = numEntries;  
    pHeader->Flags = flags; 
    pHeader->KnownHeaders = pkHeaders; 

    pResponseInfo = ALLOC_MEM(sizeof(HTTP_RESPONSE_INFO));
    if(!pResponseInfo)
        return FALSE;

    ZeroMemory(pResponseInfo, sizeof(HTTP_RESPONSE_INFO));
    pResponseInfo->Type = HttpResponseInfoTypeMultipleKnownHeaders;  
    pResponseInfo->Length = sizeof(HTTP_MULTIPLE_KNOWN_HEADERS);  
    pResponseInfo->pInfo = pHeader;  

    pResponse->ResponseInfoCount = 1; 
    pResponse->pResponseInfo = pResponseInfo; 

    return TRUE;
}

void CleanupMultipleKnownHeader(
    IN PHTTP_RESPONSE pResponse
    )
{
    if(pResponse->pResponseInfo->pInfo)
        FREE_MEM(pResponse->pResponseInfo->pInfo);

    if(pResponse->pResponseInfo)
        FREE_MEM(pResponse->pResponseInfo);
    
}
   

/***************************************************************************++

Routine Description:
    The routine sends a HTTP response.

Arguments:
    hReqQueue     - Handle to the request queue.
    pRequest      - The parsed HTTP request.
    pEntityString - Response entity body.

Return Value:
    Success/Failure.

--***************************************************************************/
DWORD
Send401HttpResponse(
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest,
    __in_opt IN PSTR pEntityString
    )
{
    HTTP_RESPONSE   response;
    HTTP_DATA_CHUNK dataChunk;
    DWORD           result;
    DWORD           bytesSent;
    HTTP_KNOWN_HEADER kHeaders[NUM_SCHEMES];

    //
    // Initialize the HTTP response structure.
    //
    INITIALIZE_HTTP_RESPONSE(&response, 401,"Authentication Required");

    //
    //We will use Multiple Known Headers
    // to send schemes 
    //
    
    kHeaders[0].pRawValue = "NTLM";
    kHeaders[0].RawValueLength = (USHORT)strlen("NTLM");

    kHeaders[1].pRawValue = "NEGOTIATE";
    kHeaders[1].RawValueLength = (USHORT)strlen("NEGOTIATE");


    //
    // Add a known header.
    //
    result = AddMultipleKnownHeader(
                                    &response,
                                    HttpHeaderWwwAuthenticate , 
                                    kHeaders, 
                                    NUM_SCHEMES, 
                                    0
                                   );


    if(!result)
    {
        //Send Error response back
        wprintf(L"Send 503 Service Unavailable\n");

        SendHttpResponse(
                         hReqQueue,
                         pRequest,
                         HTTP_SEND_RESPONSE_FLAG_DISCONNECT,
                         503,
                         "Unavailable",
                         "Resource Unavailable\n"
                        );

        goto END;
    }

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

END:

    CleanupMultipleKnownHeader(&response);

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
    WCHAR           szTempName[MAX_PATH + 1];
#define MAX_ULONG_STR ((ULONG) sizeof("4294967295"))
    CHAR            szContentLength[MAX_ULONG_STR];
    HTTP_DATA_CHUNK dataChunk;
    ULONG           TotalBytesRead = 0;
    
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



/***************************************************************************++

Routine Description:
    The routine extracts the Auth Info from the request and returns it to the caller.

Arguments:
    pRequest      - The parsed HTTP request.

Return Value:
    Success/Failure.

--***************************************************************************/
HTTP_REQUEST_AUTH_INFO *
GetAuthInfo(
    IN PHTTP_REQUEST pRequest
    )
{
    HTTP_REQUEST_AUTH_INFO *pAuthInfo = NULL;

    // Lets grab the request Info structure
    //
    if (pRequest->RequestInfoCount != 0)
    {
        ULONG index;
        //
        //Loop through all the RequestInfo's and find the auth one
        //

        for(index = 0; index < pRequest->RequestInfoCount; index++)
        {           
            if ((pRequest->pRequestInfo[index]).InfoType == HttpRequestInfoTypeAuth)
            {
                pAuthInfo = (PHTTP_REQUEST_AUTH_INFO) (pRequest->pRequestInfo[index]).pInfo;

                break;
            }
        }
    }
    else {
        wprintf(L"Did not get any Request Info Count!\n");
        goto fail_return;
    }

fail_return:

    return pAuthInfo ;
}


/***************************************************************************++

Routine Description:
    The routine handles the authentication request based on the auth status in AuthInfo

Arguments:
    pAuthInfo      - Auth Info sent by the caller.

Return Value:
    Success/Failure.

--***************************************************************************/
DWORD
HandleAuthRequest(
    IN PHTTP_REQUEST_AUTH_INFO pAuthInfo,
    IN HANDLE hReqQueue,
    IN PHTTP_REQUEST pRequest
    )
{
    ULONG  result;
    CtxtHandle hContext;
    SECURITY_STATUS secStatus;
    PCSTR ErrorStr = NULL;

    //
    // Look at the status first...
    //

    switch (pAuthInfo->AuthStatus) {

        case HttpAuthStatusSuccess:

            //
            // Use the serialized context (if need to).
            //

            result = UseSerializedContext(pAuthInfo, &hContext);

            if(NO_ERROR != result)
            {				
                wprintf(L"UseSerializedContext failed %d?!\n", result);
                return result;
            }

            //
            // Use the access token.
            //

            result = UseAccessToken(pAuthInfo);

            if(NO_ERROR != result)
            {
                wprintf(L"UseAccessToken failed %d?!\n", result);
                secStatus = DeleteSecurityContext(&hContext);
                if(SEC_E_OK != secStatus)
                {
                    wprintf(L"delete security context failed %d?!\n", secStatus);
                }
                return result;
            }

            //
            // Done with the token so let it go.
            //

            CloseHandle(pAuthInfo->AccessToken);

            secStatus = DeleteSecurityContext(&hContext);
            if(SEC_E_OK != secStatus)
            {
                wprintf(L"deleting the security context failed %d?!\n", secStatus);
            }

            wprintf(L"Send 200 OK\n");

            result = SendHttpResponse(
                hReqQueue,
                pRequest,
                0,
                200,
                "200 OK",
                "Authentication Successful\n"
                );

            if(NO_ERROR != result)
            {
                wprintf(L"Failed to send 200 OK Response\n");
                return result;
            }        

            break;

        case HttpAuthStatusNotAuthenticated : 

            wprintf(L"Send 401 Response\n");

            result = Send401HttpResponse(
                hReqQueue,
                pRequest,
                "Send Credentials\n"
                );

            if(NO_ERROR != result)
            {
                wprintf(L"Failed to send 401 Response\n");
                return result;
            }        

            break;

        case HttpAuthStatusFailure :
            
            ErrorStr = MapSecurityErrorToString(pAuthInfo->SecStatus);
            wprintf(L"Authentication failed with error %hS\n", ErrorStr);

            //
            //Send back 401
            //
            wprintf(L"Send 401 Response\n");

            result = Send401HttpResponse(
                hReqQueue,
                pRequest,
                "Send Credentials\n"
                );

            if(NO_ERROR != result)
            {
                wprintf(L"Failed to send 401 Response\n");
                return result;
            }

            break;

        default :

            wprintf(L"Bogus AuthStatus %d\n",pAuthInfo->AuthStatus);
            return ERROR_INVALID_DATA;
    }

    return result;
}


/***************************************************************************++

Routine Description:
    The routine demonstrates the use of serialized context received by app

Arguments:
    pAuthInfo      - Auth Info sent by the caller.

Return Value:
    Success/Failure.

--***************************************************************************/
DWORD
UseSerializedContext(
    IN  PHTTP_REQUEST_AUTH_INFO pAuthInfo,
    IN OUT PCtxtHandle phContext
    )
{

    SECURITY_STATUS SecStatus;
    SecBuffer       SecBuffer;

    //
    // import back.the context handle
    //

    SecBuffer.BufferType = pAuthInfo->PackedContextType;
    SecBuffer.cbBuffer   = pAuthInfo->PackedContextLength;
    SecBuffer.pvBuffer   = pAuthInfo->PackedContext;

    SecInvalidateHandle(phContext);

    SecStatus = ImportSecurityContext(
                                      pAuthInfo->pPackageName,
                                      &SecBuffer,
                                      NULL,
                                      phContext);
    
    if (SecStatus != SEC_E_OK) {

        wprintf(L"importing security context failed with %d\n",SecStatus);
        return ERROR_INVALID_DATA;
    }

    return NO_ERROR;

}

/***************************************************************************++

Routine Description:
    The routine demonstrates the use of Access token received by app

Arguments:
    pAuthInfo      - Auth Info sent by the caller.

Return Value:
    Success/Failure.

--***************************************************************************/
DWORD
UseAccessToken(
    IN  PHTTP_REQUEST_AUTH_INFO pAuthInfo
    )
{
    DWORD Error = NO_ERROR;
    WCHAR UserName[MAX_USERNAME_LENGTH];
    ULONG UserNameLen = MAX_USERNAME_LENGTH;
    
    //
    // Impersonate the client
    //

    if ( FALSE == ImpersonateLoggedOnUser(pAuthInfo->AccessToken) )
    {
        Error = GetLastError();
        wprintf(L"Failed to impersonate user! with error %d\n",Error);
        return Error;
     }

    //
    // Generate response based on the identity of the user.
    // This is just a sample to demonstrate how AccessToken
    // can be used.
    //

    if ( FALSE == GetUserName(UserName, &UserNameLen) )
    {
        Error = GetLastError();
        wprintf(
            L"Unable to get the user name for AuthType %d Error %d\n",
            pAuthInfo->AuthType,
            Error);

         goto revert;
    }

    wprintf(L"Impersonating user:%s\n", 
            UserName);

revert:
    
    if ( FALSE == RevertToSelf() )
    {
        Error = GetLastError();
        wprintf(
            L"Unable to revert back to myself after impersonating Error %d\n",
            Error);
    }


     return Error;
}
