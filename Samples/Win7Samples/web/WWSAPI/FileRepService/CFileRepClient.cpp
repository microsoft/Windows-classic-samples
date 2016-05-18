//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#include "Service.h"
#include "process.h"
#include "string.h"
#include "strsafe.h"
#include "stdlib.h"
#include "intsafe.h"
#include "assert.h"
#include "wtypes.h"

// This file contains the client-service specific code.

// The client version of ProcessMessage. This is the entry point for the application-specific code.
HRESULT CFileRepClient::ProcessMessage(
    __in CRequest* request, 
    __in const WS_XML_STRING* receivedAction)
{
    PrintVerbose(L"Entering CFileRepClient::ProcessMessage");

    HRESULT hr = S_OK;

    WS_CHANNEL* channel = request->GetChannel();
    WS_MESSAGE* requestMessage = request->GetRequestMessage();
    WS_ERROR* error = request->GetError();

     // Make sure action is what we expect
    if (WsXmlStringEquals(receivedAction, &faultAction, error) == S_OK)
    {
        PrintInfo(L"Received fault message. Aborting.");

        hr = E_FAIL;
        EXIT_FUNCTION
    }

    // Make sure action is what we expect
    if (WsXmlStringEquals(receivedAction, &userRequestAction, error) != S_OK)
    {
        PrintInfo(L"Received unexpected message");

        hr = WS_E_ENDPOINT_ACTION_NOT_SUPPORTED;
        EXIT_FUNCTION
    }

    // Get the heap of the message
    WS_HEAP* heap;
    IfFailedExit(WsGetMessageProperty(requestMessage, WS_MESSAGE_PROPERTY_HEAP, &heap, sizeof(heap), error));

    // Read user request
    UserRequest* userRequest = NULL;
    IfFailedExit(WsReadBody(requestMessage, &userRequestElement, WS_READ_REQUIRED_POINTER, heap, &userRequest, sizeof(userRequest), error));

    // Read end of message
    IfFailedExit(WsReadMessageEnd(channel, requestMessage, NULL, error));

    // Sanity check
    if (::wcslen(userRequest->sourcePath) >= MAX_PATH ||
        ::wcslen(userRequest->destinationPath) >= MAX_PATH ||
        ::wcslen(userRequest->serverUri) >= MAX_PATH)
    {
        PrintInfo(L"Invalid request");
        hr = request->SendFault(INVALID_REQUEST);
    }
    else
    {
        hr = ProcessUserRequest(request, userRequest->sourcePath,
            userRequest->destinationPath, userRequest->serverUri, userRequest->serverProtocol,
            userRequest->securityMode, userRequest->messageEncoding, userRequest->requestType);
    }

    EXIT

    // The caller handles the failures. So just return the error;
    PrintVerbose(L"Leaving CCFileRepClient::ProcessMessage");

    return hr;
}

// The message exchange pattern when transfering a file is as follows:
// - We get a request message from the command line tool
// - If the request is asynchronous send back a confirmation immediately
// - We send a request for file information to the server service. A discovery request is denoted by a chunk position of -1.
// - We get the file information
// - We request the individual chunks sequentially one by one from the server.
// Chunks are identified by their position within the file. This could be optimized by asynchronously requesting
// multiple chunks. However, that is beyond the scope of this version of the sample.
// - Repeat until the file transfer is completed or a failure occured
// - If the request is synchronous send success or failure message to the command line tool.
// For the individual data structures associated with each message, see common.h.
HRESULT CFileRepClient::ProcessUserRequest(
    __in CRequest* request,
    __in_z const LPWSTR sourcePath, 
    __in_z const LPWSTR destinationPath,
    __in_z const LPWSTR serverUri, 
    __in TRANSPORT_MODE transportMode, 
    __in SECURITY_MODE securityMode,
    __in MESSAGE_ENCODING encoding, 
    __in REQUEST_TYPE requestType)
{
    PrintVerbose(L"Entering CFileRepClient::ProcessUserRequest");

    HRESULT hr = S_OK;
    WS_ERROR* error = request->GetError();
    WS_CHANNEL* serverChannel = NULL;
    WS_MESSAGE* serverRequestMessage = NULL;
    WS_MESSAGE* serverReplyMessage = NULL;
    WS_HEAP* heap = NULL;
    HANDLE file = INVALID_HANDLE_VALUE;
    LPWSTR statusMessage = NULL;
    LONGLONG fileLength = 0;
    long chunkSize = -1;
    DWORD transferTime = 0;
    LARGE_INTEGER size;
    size.QuadPart = 0;
    WS_MESSAGE_PROPERTY heapProperty;

    WS_ENDPOINT_ADDRESS address = {};

    if (ASYNC_REQUEST == requestType)
    {
        // In case of an async request, acknowlege request before doing any actual work.
        IfFailedExit(SendUserResponse(request, TRANSFER_ASYNC));
        PrintInfo(L"Asynchronous request. Sending asynchronous acknowledgement.");
    }

    heapProperty = CFileRepClient::CreateHeapProperty();

    IfFailedExit(CreateServerChannel(encoding, transportMode, securityMode, error, &serverChannel));
    IfFailedExit(WsCreateMessageForChannel(serverChannel, NULL, 0, &serverRequestMessage, error));
    IfFailedExit(WsCreateMessageForChannel(serverChannel, &heapProperty, 1, &serverReplyMessage, error));

    // Initialize address of service
    address.url.chars = serverUri;
    IfFailedExit(SizeTToULong(::wcslen(address.url.chars),&address.url.length));

    // Open channel to address
    IfFailedExit(WsOpenChannel(serverChannel, &address, NULL, error));

     // Initialize file request
    FileRequest fileRequest;
    fileRequest.filePosition = DISCOVERY_REQUEST;
    fileRequest.fileName = sourcePath;

    // We ensured that those are not too long earlier
    SIZE_T strLen = ::wcslen(fileRequest.fileName) + address.url.length + 100;

    statusMessage = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, strLen * sizeof (WCHAR));

    // We do not care about the failure value since in the worst case it truncates, which we
    // still would want to display since its better than nothing.
    StringCchPrintfW(statusMessage, strLen, L"Requesting file %s from server %s.", fileRequest.fileName, address.url.chars);

    statusMessage[strLen-1] = L'\0'; // Terminate string in case StringCchPrintfW fails.
    PrintInfo(statusMessage);

    IfFailedExit(WsCreateHeap(65536, 0, NULL, 0, &heap, NULL));

    WS_MESSAGE_DESCRIPTION fileRequestMessageDescription;
    fileRequestMessageDescription.action = &fileRequestAction;
    fileRequestMessageDescription.bodyElementDescription = &fileRequestElement;

    WS_MESSAGE_DESCRIPTION fileInfoMessageDescription;
    fileInfoMessageDescription.action = &fileInfoAction;
    fileInfoMessageDescription.bodyElementDescription = &fileInfoElement;

    // Send discovery request and get file info
    FileInfo* fileInfo;
    IfFailedExit(WsRequestReply(
        serverChannel,
        serverRequestMessage,
        &fileRequestMessageDescription,
        WS_WRITE_REQUIRED_VALUE,
        &fileRequest,
        sizeof(fileRequest),
        serverReplyMessage,
        &fileInfoMessageDescription,
        WS_READ_REQUIRED_POINTER,
        heap,
        &fileInfo,
        sizeof(fileInfo),
        NULL,
        error));

    fileLength = fileInfo->fileLength;
    chunkSize = fileInfo->chunkSize;
    size.QuadPart = fileLength;

    if (-1 == fileLength || 0 == chunkSize)
    {
        PrintInfo(L"File does not exist on server.");
        if (SYNC_REQUEST == requestType)
        {
            hr = request->SendFault(FILE_DOES_NOT_EXIST);
        }

        EXIT_FUNCTION
    }

    // For simplicity reasons we do not read alternate data streams.
    file = CreateFileW(destinationPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == file)
    {
        PrintInfo(L"Failed to create file");

        if (SYNC_REQUEST == requestType)
        {
            hr = request->SendFault(FAILED_TO_CREATE_FILE);
        }
        EXIT_FUNCTION
    }

    IfFailedExit(ExtendFile(file, fileLength));

    transferTime = GetTickCount();

    // This loop could be further optimized by asychronously requesting the individual chunks in
    // parallel and then assembling them later. We chose to draw the line regarding perf optimizations
    // for this version of the code here to avoid the complexity of doing asynchronous file assembly.

    fileRequest.filePosition = 0;
    while (fileRequest.filePosition < fileLength)
    {
        IfFailedExit(ProcessChunk(chunkSize , file, fileLength, serverRequestMessage,
            serverReplyMessage, serverChannel, error, &fileRequest));
    }

    transferTime = GetTickCount() - transferTime;

    if (SYNC_REQUEST == requestType)
    {
        hr = SendUserResponse(request, TRANSFER_SUCCESS);
    }

    WCHAR perf[255];
    // This assumes that we did not use more than 4 billion chunks, a pretty reasonable assumption.
    DWORD totalChunks = (DWORD)(fileLength/chunkSize) + 1;
    if (size.HighPart != 0) // Big file
    {
        // Again failures are ignored since it is just a status message.
        StringCchPrintfW(perf, CountOf(perf), L"Transferred %d%d bytes via %d chunks in %d milliseconds.",
            size.HighPart, size.LowPart, totalChunks, transferTime);
    }
    else
    {
        StringCchPrintfW(perf, CountOf(perf), L"Transferred %d bytes via %d chunks in %d milliseconds.",
            size.LowPart, totalChunks, transferTime);
    }

    perf[CountOf(perf)-1] = L'\0'; // Ensures that the buffer is terminated even if StringCChPrintfW fails.

    PrintInfo(perf);

    // We fall through here even in the success case due to the cleanup that has to be performed in both cases.
    EXIT

    //Has to come first so file gets properly deleted on error.
    if (INVALID_HANDLE_VALUE != file)
    {
        if (!CloseHandle(file))
        {
            PrintInfo(L"CFileRepClient::ProcessUserRequest - CloseHandle failed. Potential handle leak.");
        }
    }

    if (NULL != serverRequestMessage)
    {
        WsFreeMessage(serverRequestMessage);
    }
    if (NULL != serverReplyMessage)
    {
        WsFreeMessage(serverReplyMessage);
    }

    CleanupChannel(serverChannel);

    if (NULL != statusMessage)
    {
        HeapFree(GetProcessHeap(), 0, statusMessage);
    }

    if (NULL != heap)
    {
        WsFreeHeap(heap);
    }

    if (FAILED(hr))
    {
        DeleteFileW(destinationPath);

        PrintError(L"CFileRepClient::ProcessUserRequest", true);
        PrintError(hr, error, true);
    }

    PrintVerbose(L"Leaving CFileRepClient::ProcessUserRequest");

    return hr;
}

HRESULT CFileRepClient::CreateServerChannel(
    __in MESSAGE_ENCODING serverEncoding, 
    __in TRANSPORT_MODE serverTransportMode,
    __in SECURITY_MODE serverSecurityMode, 
    __in_opt WS_ERROR* error, 
    __deref_out WS_CHANNEL** channel)
{
    PrintVerbose(L"Entering CFileRepClient::CreateServerChannel");

    WS_SSL_TRANSPORT_SECURITY_BINDING transportSecurityBinding = {};
    WS_SECURITY_DESCRIPTION securityDescription = {};
    WS_SECURITY_DESCRIPTION* pSecurityDescription = NULL;
    WS_SECURITY_BINDING* securityBindings[1];
    HRESULT hr = S_OK;

    WS_CHANNEL_PROPERTY channelProperty[2];

    // The default maximum message size is 64k. This is not enough as the server may chose to send
    // very large messages to maximize throughput.
    ULONG maxMessageSize = MAXMESSAGESIZE;
    channelProperty[0].id = WS_CHANNEL_PROPERTY_MAX_BUFFERED_MESSAGE_SIZE;
    channelProperty[0].value = &maxMessageSize;
    channelProperty[0].valueSize = sizeof(maxMessageSize);

    WS_ENCODING messageEncoding;
    if (TEXT_ENCODING == serverEncoding)
    {
        messageEncoding = WS_ENCODING_XML_UTF8;
    }
    else if (BINARY_ENCODING == serverEncoding)
    {
        messageEncoding = WS_ENCODING_XML_BINARY_SESSION_1;
    }
    else
    {
        messageEncoding = WS_ENCODING_XML_MTOM_UTF8;
    }

    channelProperty[1].id = WS_CHANNEL_PROPERTY_ENCODING;
    channelProperty[1].value = &messageEncoding;
    channelProperty[1].valueSize = sizeof(messageEncoding);

    ULONG propCount = 1;
    if (DEFAULT_ENCODING != serverEncoding)
    {
        propCount++;
    }

    if (SSL_SECURITY == serverSecurityMode)
    {
        // Initialize a security description for SSL
        transportSecurityBinding.binding.bindingType = WS_SSL_TRANSPORT_SECURITY_BINDING_TYPE;
        securityBindings[0] = &transportSecurityBinding.binding;
        securityDescription.securityBindings = securityBindings;
        securityDescription.securityBindingCount = WsCountOf(securityBindings);
        pSecurityDescription = &securityDescription;
    }

    // Create a channel
    if (TCP_TRANSPORT == serverTransportMode)
    {
        hr = WsCreateChannel(WS_CHANNEL_TYPE_DUPLEX_SESSION, WS_TCP_CHANNEL_BINDING, channelProperty,
            propCount, pSecurityDescription, channel, error);
    }
    else // HTTP
    {
        hr = WsCreateChannel(WS_CHANNEL_TYPE_REQUEST, WS_HTTP_CHANNEL_BINDING,
            channelProperty, propCount, pSecurityDescription, channel, error);
    }

    PrintVerbose(L"Leaving CFileRepClient::CreateServerChannel");

    return hr;
}

// Extend the file to the total size needed for performance reasons.
// For a synchronous write such as this this is not a big deal, but if this code
// would be made async then it would be. And even now this is more performant.
HRESULT CFileRepClient::ExtendFile(
    __in HANDLE file, 
    __in LONGLONG length)
{
    LARGE_INTEGER size;
    size.QuadPart = length;

    if (!SetFilePointerEx(file, size, NULL, FILE_BEGIN))
    {
        PrintError(L"Error extending file.", true);
        return HRESULT_FROM_WIN32(GetLastError());

    }
    if (!SetEndOfFile(file))
    {
        PrintError(L"Error extending file.", true);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    size.QuadPart = 0;

    if (!SetFilePointerEx(file, size, NULL, FILE_BEGIN))
    {
        PrintError(L"Error resetting file.", true);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HRESULT CFileRepClient::ProcessChunk(
    __in long chunkSize, 
    __in HANDLE file, 
    __in LONGLONG fileLength, 
    __in WS_MESSAGE* requestMessage,
    __in WS_MESSAGE* replyMessage, 
    __in WS_CHANNEL* channel, 
    __in_opt WS_ERROR* error, 
    __in FileRequest* request)
{
    PrintVerbose(L"Entering CFileRepClient::ProcessChunk");

    LONGLONG pos = request->filePosition;
    LONGLONG chunkPosition = 0;
    long contentLength = 0;
    HRESULT hr = S_OK;

    IfFailedExit(WsResetMessage(requestMessage, error));
    IfFailedExit(WsResetMessage(replyMessage, error));

    WS_MESSAGE_DESCRIPTION fileRequestMessageDescription;
    fileRequestMessageDescription.action = &fileRequestAction;
    fileRequestMessageDescription.bodyElementDescription = &fileRequestElement;

    IfFailedExit(WsSendMessage(
        channel,
        requestMessage,
        &fileRequestMessageDescription,
        WS_WRITE_REQUIRED_VALUE,
        request,
        sizeof(*request),
        NULL,
        error));

    // Receive start of message (headers).
    IfFailedExit(WsReadMessageStart(channel, replyMessage, NULL, error));

    // Get action value.
    WS_XML_STRING* receivedAction = NULL;
    IfFailedExit(WsGetHeader(
        replyMessage,
        WS_ACTION_HEADER,
        WS_XML_STRING_TYPE,
        WS_READ_REQUIRED_POINTER,
        NULL,
        &receivedAction,
        sizeof(receivedAction),
        error));
    // Make sure action is what we expect.
    if (WsXmlStringEquals(receivedAction, &fileReplyAction, error) != S_OK)
    {
        hr = WS_E_ENDPOINT_ACTION_NOT_SUPPORTED;
        PrintInfo(L"Received unexpected message.\n");

        EXIT_FUNCTION
    }

    IfFailedExit(DeserializeAndWriteMessage(replyMessage, chunkSize, &chunkPosition, &contentLength, file));

    // Read end of message.
    IfFailedExit(WsReadMessageEnd(channel, replyMessage, NULL, error));

    if (contentLength != chunkSize && contentLength + pos != fileLength || pos != chunkPosition)
    {
        PrintError(L"File message was corrupted. Aborting transfer\n", true);
        hr = E_FAIL;
    }
    else
    {
        request->filePosition = pos + contentLength;
    }


    EXIT

    if (WS_E_INVALID_FORMAT == hr)
    {
        PrintInfo(L"Deserialization of the message failed.");
    }

    PrintVerbose(L"Leaving CFileRepClient::ProcessChunk");

    return hr;
}

// It is more efficient to manually read this message instead of using the serializer since otherwise the
// byte array would have to be copied around memory multiple times while here we can stream it directly into the file.
// Since the message is simple this is relatively easy to do and makes the perf gain worth the extra effort. In general,
// one should only go down to this level if the performance gain is significant. For most cases the serialization APIs
// are the better choice and they also make future changes easier to implement.
HRESULT CFileRepClient::DeserializeAndWriteMessage(
    __in WS_MESSAGE* message, 
    __in long chunkSize,
    __out LONGLONG* chunkPosition, 
    __out long* contentLength, 
    __in HANDLE file)
{
    PrintVerbose(L"Entering CFileServer::DeserializeAndWriteMessage");
    WS_XML_READER* reader = NULL;
    HRESULT hr = S_OK;
    LPWSTR errorString = NULL;
    WS_HEAP* heap = NULL;

    // Create a description for the error text field that we read later.
    WS_ELEMENT_DESCRIPTION errorDescription = {&errorLocalName, &fileChunkNamespace, WS_WSZ_TYPE, NULL};

    // To avoid using too much memory we read and write the message in chunks.
    long bytesToRead = FILE_CHUNK;
    if (bytesToRead > chunkSize)
    {
        bytesToRead = chunkSize;
    }

    BYTE* buf = NULL;
    ULONG length = 0;

    // We do not use WS_ERROR here since this function does not print extended error information.
    // The reason for that is that a failure here is likely due to a malformed message coming in, which
    // is probably a client issue and not an error condition for us. So the caller will print a simple status
    // message if this fails and call it good.
    IfFailedExit(WsGetMessageProperty(message, WS_MESSAGE_PROPERTY_BODY_READER, &reader, sizeof(reader), NULL));

    // Read to FileChunk element
    IfFailedExit(WsReadToStartElement(reader, &fileChunkLocalName, &fileChunkNamespace, NULL, NULL));

    // Read FileChunk start element
    IfFailedExit(WsReadStartElement(reader, NULL));

    // The next four APIs read the chunk position element. There are helper APIs that can do this for you,
    // but they are more complex to use. For simple types such as integers, doing this manually is easiest.
    // For more complex type the serialization APIs should be used.

    // Read to chunkPosition element
    IfFailedExit(WsReadToStartElement(reader, &chunkPositionLocalName, &fileChunkNamespace, NULL, NULL));

    // Read chunk position start element
    IfFailedExit(WsReadStartElement(reader, NULL));

    // Read chunk position
    IfFailedExit(WsReadValue(reader, WS_INT64_VALUE_TYPE, chunkPosition, sizeof(*chunkPosition), NULL));

    // Read chunk position end element
    IfFailedExit(WsReadEndElement(reader, NULL));

    // Read to file content element
    IfFailedExit(WsReadToStartElement(reader, &fileContentLocalName, &fileChunkNamespace, NULL, NULL));

    // Read file content start element
    IfFailedExit(WsReadStartElement(reader, NULL));

    buf = (BYTE*)HeapAlloc(GetProcessHeap(), 0, bytesToRead);
    IfNullExit(buf);

    // Read file content into buffer
    // We are reading a chunk of the byte array in the message, writing it to disk and then read
    // the next chunk. That way we only need mimimal amounts of memory compared to the total amount
    // of data transferred. The exact way this is done is subject to perf tweaking.
    for (;;)
    {
        // Read next block of bytes.
        ULONG bytesRead = 0;
        IfFailedExit(WsReadBytes(reader, buf, bytesToRead, &bytesRead, NULL));

        if (bytesRead == 0)
        {
            // We read it all.
            break;
        }

        length+=bytesRead;

        ULONG count = 0;

        if (!WriteFile(file, buf, bytesRead, &count, NULL))
        {
            PrintError(L"File write error.", true);
            hr = HRESULT_FROM_WIN32(GetLastError());
            EXIT_FUNCTION
        }

        if (count != bytesRead)
        {
            PrintError(L"File write error.", true);
            hr = E_FAIL;
            EXIT_FUNCTION
        }
    }

    // Read file content end element
    IfFailedExit(WsReadEndElement(reader, NULL));

     // Read the error string and write it to a heap.
    IfFailedExit(WsCreateHeap(/*maxSize*/ 1024, /*trimSize*/ 1024, NULL, 0, &heap, NULL));

    // Here it pays to use WsReadElementType instead of manually parsing the element since we require heap memory anyway.
    // This can fail if the error message does not fit on the relatively small heap created for it. The server is not
    // expected to use error messages that long. If we get back such a long message we talk to a buggy or rogue server
    // and failing is the right thing to do.
    IfFailedExit(WsReadElement(reader, &errorDescription, WS_READ_REQUIRED_POINTER, heap,
        &errorString, sizeof(errorString), NULL));
    // Read file data end element
    IfFailedExit(WsReadEndElement(reader, NULL));

    *contentLength = length;

    if (lstrcmpW(errorString, &GlobalStrings::noError[0]))
    {
        PrintInfo(L"Chunk transfer failed");
        if (errorString)
        {
            PrintInfo(errorString);
        }
        hr = E_FAIL;
    }


    EXIT

    if (NULL != buf)
    {
        HeapFree(GetProcessHeap(), 0, buf);
    }

    if (heap)
    {
        // Clean up errorString.
        WsResetHeap(heap, NULL);
        WsFreeHeap(heap);
    }

    if (FAILED(hr))
    {
        hr = WS_E_INVALID_FORMAT;
    }

    PrintVerbose(L"Leaving CFileRepClient::DeserializeAndWriteMessage");

    return hr;
}

// Tell the command line tool what happened to the request.
HRESULT CFileRepClient::SendUserResponse(
    __in CRequest* request, 
    __in TRANSFER_RESULTS result)
{
    PrintVerbose(L"Entering CFileRepClient::SendUserResponse");

    HRESULT hr = S_OK;
    WS_ERROR* error = request->GetError();
    WS_MESSAGE* replyMessage = request->GetReplyMessage();
    WS_MESSAGE* requestMessage = request->GetRequestMessage();
    WS_CHANNEL* channel = request->GetChannel();

    UserResponse userResponse;
    userResponse.returnValue = result;

    WS_MESSAGE_DESCRIPTION userResponseMessageDescription;
    userResponseMessageDescription.action = &userResponseAction;
    userResponseMessageDescription.bodyElementDescription = &userResponseElement;

    hr = WsSendReplyMessage(
        channel,
        replyMessage,
        &userResponseMessageDescription,
        WS_WRITE_REQUIRED_VALUE,
        &userResponse,
        sizeof(userResponse),
        requestMessage,
        NULL, error);
    if (FAILED(hr))
    {
        PrintError(L"CFileRepClient::SendUserResponse\n", true);
        PrintError(hr, error, true);
    }

    WsResetMessage(replyMessage, NULL);

    PrintVerbose(L"Leaving CFileRepClient::SendUserResponse");

    return hr;
}


