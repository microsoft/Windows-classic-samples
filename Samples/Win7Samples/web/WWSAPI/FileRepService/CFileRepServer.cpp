//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

// This file contains the server service specific code.

#include "Service.h"
#include "assert.h"

// The server version of ProcessMessage. This is the entry point for the application-specific code.
HRESULT CFileRepServer::ProcessMessage(
    __in CRequest* request,
    __in const WS_XML_STRING* receivedAction)
{
    PrintVerbose(L"Entering CFileRepServer::ProcessMessage");

    HRESULT hr = S_OK;
    FileRequest* fileRequest = NULL;
    WS_MESSAGE* requestMessage = request->GetRequestMessage();
    WS_CHANNEL* channel = request->GetChannel();
    WS_ERROR* error = request->GetError();

    // Make sure action is what we expect
    if (WsXmlStringEquals(receivedAction, &fileRequestAction, error) != S_OK)
    {
        PrintInfo(L"Illegal action");

        hr = WS_E_ENDPOINT_ACTION_NOT_SUPPORTED;
    }
    else
    {
        // Read file request

        WS_HEAP* heap;
        IfFailedExit(WsGetMessageProperty(requestMessage, WS_MESSAGE_PROPERTY_HEAP, &heap, sizeof(heap), error));

        IfFailedExit(WsReadBody(requestMessage, &fileRequestElement, WS_READ_REQUIRED_POINTER,
            heap, &fileRequest, sizeof(fileRequest), error));
        IfFailedExit(WsReadMessageEnd(channel, requestMessage, NULL, error));

        IfFailedExit(ReadAndSendFile(request, fileRequest->fileName, fileRequest->filePosition, error));
    }

    EXIT

    // We do not print error messages here. That is handled in the caller.
    PrintVerbose(L"Leaving CFileRepServer::ProcessMessage");
    return hr;
}

// Performs the actual file transfer.
// This function will fail to produce a valid file on the client if the file was changed in between requests for chunks.
// There are ways to work around that, but doing so is beyond the scope of this version of the sample. A simple fix would be
// to keep the file open between requests and prevent writing, but in the spirit of web services this app does not maintain
// state between requests.
HRESULT CFileRepServer::ReadAndSendFile(
    __in CRequest* request, 
    __in const LPWSTR fileName, 
    __in LONGLONG chunkPosition, 
    __in_opt WS_ERROR* error)
{
    PrintVerbose(L"Entering CFileRepServer::ReadAndSendFile");

    HANDLE file = NULL;
    HRESULT hr = S_OK;

    file = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == file)
    {
        PrintInfo(L"Invalid file name");
        if (-1 != chunkPosition)
        {
            hr = SendError(request, GlobalStrings::invalidFileName);
        }
        else
        {
            hr = SendFileInfo(request, fileName, -1, chunkSize);
        }

        PrintVerbose(L"Leaving CFileRepServer::ReadAndSendFile");
        return hr;
    }

    LARGE_INTEGER len;

    if (!GetFileSizeEx(file, &len))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        PrintError(L"Unable to determine file length", true);
        if (FAILED(SendError(request, GlobalStrings::unableToDetermineFileLength)))
        {
            PrintError(L"Unable to send failure back", true);
        }

        if (!CloseHandle(file))
        {
            PrintError(L"Unable to close file handle", true);
        }

        // This has its own return path to ensure that the right error info is returned.
        // The main error path would overwrite it.
        PrintVerbose(L"Leaving CFileRepServer::ReadAndSendFile");
        return hr;
    }

    LONGLONG fileLength = len.QuadPart;

    if (chunkPosition == DISCOVERY_REQUEST)
    {
        PrintInfo(L"Processing discovery message");
        hr = SendFileInfo(request, fileName, fileLength, chunkSize);
    }
    else if (chunkPosition < -1)
    {
        PrintInfo(L"Invalid request");
        hr = SendError(request, GlobalStrings::invalidRequest);
    }
    else if (chunkPosition >= fileLength)
    {
        PrintInfo(L"Request out of range of the file");
        hr = SendError(request, GlobalStrings::outOfRange);
    }
    else
    {
        long chunkSize = this->chunkSize;
        if (fileLength - chunkPosition < chunkSize)
        {
            chunkSize = (DWORD)(fileLength - chunkPosition);
        }

        LARGE_INTEGER pos;
        pos.QuadPart = chunkPosition;

        if (!SetFilePointerEx(file, pos, NULL, FILE_BEGIN))
        {
            PrintError(L"Unable to set file pointer", true);
            hr = E_FAIL;

            // Ignore return value as we already have a failure.
            SendError(request, GlobalStrings::unableToSetFilePointer);

        }
        else
        {
            hr = ReadAndSendChunk(request, chunkSize, chunkPosition, file);
        }
    }

    if (FAILED(hr))
    {
        PrintError(L"CFileRepServer::ReadAndSendFile\n", true);
        PrintError(hr, error, true);
    }

    if (!CloseHandle(file))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        PrintError(L"Unable to close file handle", true);
    }

    PrintVerbose(L"Leaving CFileRepServer::ReadAndSendFile");
    return hr;
}

// The first message of a file transfer is a discovery request byt the client. This function handles such requests.
HRESULT CFileRepServer::SendFileInfo(
    __in CRequest* request, 
    __in_z const LPWSTR fileName, 
    __in LONGLONG fileLength, 
    __in DWORD chunkSize)
{
    PrintVerbose(L"Entering CFileRepServer::SendFileInfo");

    HRESULT hr = S_OK;
    WS_ERROR* error = request->GetError();
    WS_MESSAGE* replyMessage = request->GetReplyMessage();
    WS_MESSAGE* requestMessage = request->GetRequestMessage();
    WS_CHANNEL* channel = request->GetChannel();

    FileInfo fileInfo;
    fileInfo.fileName = fileName;
    fileInfo.fileLength = fileLength;
    fileInfo.chunkSize = chunkSize;

    WS_MESSAGE_DESCRIPTION fileInfoMessageDescription;
    fileInfoMessageDescription.action = &fileInfoAction;
    fileInfoMessageDescription.bodyElementDescription = &fileInfoElement;

    hr = WsSendReplyMessage(
        channel,
        replyMessage,
        &fileInfoMessageDescription,
        WS_WRITE_REQUIRED_VALUE,
        &fileInfo,
        sizeof(fileInfo),
        requestMessage,
        NULL,
        error);

    if (FAILED(hr))
    {
        PrintError(L"CFileRepServer::SendFileInfo", true);
        PrintError(hr, error, true);
    }

    WsResetMessage(replyMessage, NULL);

    PrintVerbose(L"Leaving CFileRepServer::SendFileInfo");
    return hr;
}

// Reads the data and serializes it into the message. This function does custom serialization for the same
// reason and with the same alrogithm as DeserializeAndWriteMessage.
HRESULT CFileRepServer::ReadAndSendChunk(
    __in CRequest* request,
    __in long chunkSize, 
    __in LONGLONG chunkPosition,
    __in HANDLE file)
{
    PrintVerbose(L"Entering CFileRepServer::ReadAndSendChunk");

    if (chunkSize < 0 || chunkPosition < 0)
    {
        PrintVerbose(L"Leaving CFileRepServer::ReadAndSendChunk");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    WS_XML_WRITER* writer = NULL;
    WS_MESSAGE* replyMessage = request->GetReplyMessage();
    WS_MESSAGE* requestMessage = request->GetRequestMessage();
    WS_ERROR* error = request->GetError();
    WS_CHANNEL* channel = request->GetChannel();

    BYTE* buf = NULL;
    LONG length = 0;

    // To avoid using too much memory we read and write the message in chunks.
    LONG bytesToRead = FILE_CHUNK;
    if (bytesToRead > chunkSize)
    {
        bytesToRead = chunkSize;
    }

    buf = (BYTE*)HeapAlloc(GetProcessHeap(), 0, bytesToRead);
    IfNullExit(buf);

    IfFailedExit(WsInitializeMessage(replyMessage, WS_BLANK_MESSAGE, requestMessage, error));

    // Add the action header
    IfFailedExit(WsSetHeader(
        replyMessage,
        WS_ACTION_HEADER,
        WS_XML_STRING_TYPE,
        WS_WRITE_REQUIRED_VALUE,
        &fileReplyAction,
        sizeof(fileReplyAction),
        error));

    // Send the message headers
    IfFailedExit(WsWriteMessageStart(channel, replyMessage, NULL, error));

    // Get writer to serialize message body
    IfFailedExit(WsGetMessageProperty(replyMessage, WS_MESSAGE_PROPERTY_BODY_WRITER, &writer, sizeof(writer), error));

    // Write FileChunk start element.
    // This whole code block is the serialization equivalent of the desiralization code.
    IfFailedExit(WsWriteStartElement(writer, NULL, &fileChunkLocalName, &fileChunkNamespace, error));

    // Write chunkPosition element
    IfFailedExit(WsWriteStartElement(writer, NULL, &chunkPositionLocalName, &fileChunkNamespace, error));
    IfFailedExit(WsWriteValue(writer, WS_INT64_VALUE_TYPE, &chunkPosition, sizeof(chunkPosition), error));
    IfFailedExit(WsWriteEndElement(writer, error));

    // Write fileContent start element
    IfFailedExit(WsWriteStartElement(writer, NULL, &fileContentLocalName, &fileChunkNamespace, error));

    // Like in the deserialization code, we read the file in multiple steps to avoid
    // having to have everything in memory at once. The message could potentially be
    // big so this is more efficient.
    for (;;)
    {
        ULONG bytesRead = 0;

        if (length + bytesToRead > chunkSize)
        {
            bytesToRead = chunkSize - length;
        }

        if (!ReadFile(file, buf, bytesToRead, &bytesRead, NULL))
        {

            PrintError(L"File read error.", true);
            hr = HRESULT_FROM_WIN32(GetLastError());

            EXIT_FUNCTION
        }

        if (0 == bytesRead)
        {
            // We reched the end of the file before filling the chunk. Send a partial chunk.
            break;
        }

        IfFailedExit(WsWriteBytes(writer, buf, bytesRead, error));

        length += bytesRead;

        if (length == chunkSize)
        {
            // We filled the message
            break;
        }
    }

    // Write fileContent end element
    IfFailedExit(WsWriteEndElement(writer, error));

    // Write error element
    IfFailedExit(WsWriteStartElement(writer, NULL, &errorLocalName, &fileChunkNamespace, error));
    const WCHAR* noError = GlobalStrings::noError;
    IfFailedExit(WsWriteType(
        writer,
        WS_ELEMENT_TYPE_MAPPING,
        WS_WSZ_TYPE,
        NULL,
        WS_WRITE_REQUIRED_POINTER,
        &noError,
        sizeof(noError),
        error));

    // Closing elements;
    IfFailedExit(WsWriteEndElement(writer, error));
    IfFailedExit(WsWriteEndElement(writer, error));
    IfFailedExit(WsWriteMessageEnd(channel, replyMessage, NULL, error));

    hr = WsResetMessage(replyMessage, NULL);
    HeapFree(GetProcessHeap(), 0, buf);

    PrintVerbose(L"Leaving CFileRepServer::ReadAndSendChunk");
    return hr;


    ERROR_EXIT

    PrintError(L"CFileRepServer::ReadAndSendChunk", true);
    PrintError(hr, error, true);

    WsResetMessage(replyMessage, NULL);
    if (NULL != buf)
    {
        HeapFree(GetProcessHeap(), 0, buf);
    }

    PrintVerbose(L"Leaving CFileRepServer::ReadAndSendChunk");
    return hr;
}

// Construct an error message containing no data except the error string.
HRESULT CFileRepServer::SendError(
    __in CRequest* request, 
    __in_z const WCHAR errorMessage[])
{
    PrintVerbose(L"Entering CFileRepServer::SendError");

    HRESULT hr = S_OK;
    WS_ERROR* error = request->GetError();
    WS_MESSAGE* replyMessage = request->GetReplyMessage();
    WS_MESSAGE* requestMessage = request->GetRequestMessage();
    WS_CHANNEL* channel = request->GetChannel();

    FileChunk fileChunk;
    fileChunk.fileContent.bytes = NULL;
    fileChunk.fileContent.length = 0;
    fileChunk.chunkPosition = -1;
    fileChunk.error = (LPWSTR)errorMessage;

    WS_MESSAGE_DESCRIPTION fileReplyMessageDescription;
    fileReplyMessageDescription.action = &fileReplyAction;
    fileReplyMessageDescription.bodyElementDescription = &fileChunkElement;

    // As there is no large payload we use the serializer here.
    hr = WsSendReplyMessage(
        channel,
        replyMessage,
        &fileReplyMessageDescription,
        WS_WRITE_REQUIRED_VALUE,
        &fileChunk,
        sizeof(fileChunk),
        requestMessage,
        NULL,
        error);

    if (FAILED(hr))
    {
        PrintError(L"CFileRepServer::SendError\n", true);
        PrintError(hr, error, true);
    }

    WsResetMessage(replyMessage, NULL);

    PrintVerbose(L"Leaving CFileRepServer::SendError");

    return hr;
}



