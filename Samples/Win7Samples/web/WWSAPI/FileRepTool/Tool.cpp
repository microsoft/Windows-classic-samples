//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

// This file contains the code for the command line utility.

#include "common.h"
#include "assert.h"
#include "intsafe.h"

// This function closes the channel if it was openend and then frees it.
void CleanupChannel(WS_CHANNEL* channel)
{
    ULONG state = 0;

    if (NULL == channel)
    {
        return;
    }        
#ifdef DBG
    HRESULT hr = WsGetChannelProperty(channel, WS_CHANNEL_PROPERTY_STATE, &state, sizeof(state), NULL);
    assert(SUCCEEDED(hr));
#else
    (void)WsGetChannelProperty(channel, WS_CHANNEL_PROPERTY_STATE, &state, sizeof(state), NULL);
#endif

        
    if (WS_CHANNEL_STATE_OPEN == state || WS_CHANNEL_STATE_FAULTED == state)
    {
        // CloseChannel will close the channel even if it encouters an error. So ignore the error here
        // as this is called only when we destroy the channel.
        WsCloseChannel(channel, NULL, NULL);
    }

    WsFreeChannel(channel);
}

// Currently we only support 3 modes: TCP, HTTP and HTTP with SSL. Thus, this simple check here is
// enough to figure out the proper configuration. This will not be enough once we have more advanced
// security settings. The wire protocol makes these settings explicit so that it does not have
// to be changed in that case. This is only used for the command line.
// For more advanced parsing, WsDecodeUrl should be used.
HRESULT ParseTransport(
    __in const LPWSTR url, 
    __out TRANSPORT_MODE* transport, 
    __out SECURITY_MODE* securityMode)
{
    if (wcsstr(url, L"http:") == url)
    {
        *transport = HTTP_TRANSPORT;    
        *securityMode = NO_SECURITY;
        return S_OK;
    }
    else if (wcsstr(url, L"https:") == url)
    {
        *transport = HTTP_TRANSPORT;
        *securityMode = SSL_SECURITY;
        return S_OK;
    }
    else if (wcsstr(url, L"net.tcp:") == url)
    {
        *transport = TCP_TRANSPORT;
        *securityMode = NO_SECURITY;
        return S_OK;
    }

    return E_FAIL;
}

// Print out rich error info
void PrintError(
    __in HRESULT errorCode, 
    __in_opt WS_ERROR* error)
{
    wprintf(L"Failure: errorCode=0x%lx\n", errorCode);

    if (errorCode == E_INVALIDARG || errorCode == WS_E_INVALID_OPERATION)
    {
        // Correct use of the APIs should never generate these errors
        wprintf(L"The error was due to an invalid use of an API.  This is likely due to a bug in the program.\n");
        DebugBreak();
    }

    HRESULT hr = S_OK;
    if (error != NULL)
    {
        ULONG errorCount;
        hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
        if (FAILED(hr))
        {
            goto Exit;
        }
        for (ULONG i = 0; i < errorCount; i++)
        {
            WS_STRING string;
            hr = WsGetErrorString(error, i, &string);
            if (FAILED(hr))
            {
                goto Exit;
            }
            wprintf(L"%.*s\n", string.length, string.chars);
        }
    }
Exit:
    if (FAILED(hr))
    {
        wprintf(L"Could not get error string (errorCode=0x%lx)\n", hr);
    }
}

HRESULT ParseCommandLine(
    __in int argc, 
    __in_ecount(argc) wchar_t** argv, 
    __out bool* synchronous, 
    __out MESSAGE_ENCODING* messageEncoding)
{
    *synchronous = false;
    *messageEncoding = DEFAULT_ENCODING;

    for (int i = 0; i < argc; i++)
    {
        WCHAR* arg = argv[i];
        if (!_wcsicmp(arg, L"-sync") || !_wcsicmp(arg, L"/sync"))
        {
            *synchronous = true;
        }
        else if (!_wcsicmp(arg, L"-encoding:binary") || !_wcsicmp(arg, L"/encoding:binary"))
        {
            if (DEFAULT_ENCODING != *messageEncoding)
            {
                wprintf(L"Error: More than one encoding specified.\n");
                return E_FAIL;
            }

            *messageEncoding = BINARY_ENCODING;
        }
        else if (!_wcsicmp(arg, L"-encoding:text") || !_wcsicmp(arg, L"/encoding:text"))
        {
            if (DEFAULT_ENCODING != *messageEncoding)
            {
                wprintf(L"Error: More than one encoding specified.\n");
                return E_FAIL;
            }

            *messageEncoding = TEXT_ENCODING;
        }
        else if (!_wcsicmp(arg, L"-encoding:MTOM") || !_wcsicmp(arg, L"/encoding:MTOM"))
        {
            if (DEFAULT_ENCODING != *messageEncoding)
            {
                wprintf(L"Error: More than one encoding specified.\n");
                return E_FAIL;
            }

            *messageEncoding = MTOM_ENCODING;
        }
        else if (!_wcsnicmp(arg, L"-encoding:", 10) || !_wcsnicmp(arg, L"/encoding:", 10))
        {
            wprintf(L"Error: Illegal encoding specified.\n");
            return E_FAIL;
        }
        else
        {
            wprintf(L"Unrecognized parameter: %s.\n", arg);
            return E_FAIL;
        }
    }

    return S_OK;
}

int __cdecl wmain(
    __in int argc, 
    __in_ecount(argc) wchar_t** argv)
{
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_CHANNEL* channel = NULL;
    WS_MESSAGE* requestMessage = NULL;
    WS_MESSAGE* replyMessage = NULL;
    WS_HEAP* heap = NULL;

    WS_ENDPOINT_ADDRESS address = {};
    UserRequest userRequest;
    UserResponse* userResponse = NULL;  

    if (argc < 5)
    {
        wprintf(L"Usage:\n FileRep.exe <Client Service Url> <Server Service Url> <Source File> <Destination File>");
        wprintf(L"[/encoding:<binary/text/MTOM>] [/sync]\n");

        return -1;
    }

    // required arguments
    LPWSTR clientURL = argv[1];
    LPWSTR serverURL = argv[2];
    LPWSTR sourcePath = argv[3];
    LPWSTR destinationPath = argv[4];

    // optional arguments
    bool synchronous = FALSE;
    MESSAGE_ENCODING messageEncoding = DEFAULT_ENCODING;
    TRANSPORT_MODE clientTransport = HTTP_TRANSPORT;
    TRANSPORT_MODE serverTransport = TCP_TRANSPORT;
    SECURITY_MODE clientSecurityMode = NO_SECURITY;
    SECURITY_MODE serverSecurityMode = NO_SECURITY;

    if (argc > 5)
    {        
        hr = ParseCommandLine(argc - 5, &argv[5], &synchronous, &messageEncoding);
        if (FAILED(hr))
        {
            return -1;
        }
    }  
        
    if (synchronous)
    {
        wprintf(L"Performing synchronous request.\n");
    }
    else
    {
        wprintf(L"Performing asynchronous request.\n");
    }
    if (DEFAULT_ENCODING == messageEncoding)
    {
        wprintf(L"Using default encoding.\n");
    }
    else if (TEXT_ENCODING == messageEncoding)
    {
        wprintf(L"Using text encoding.\n");
    }
    else if (BINARY_ENCODING == messageEncoding)
    {
        wprintf(L"Using binary encoding.\n");
    }
    else if (MTOM_ENCODING == messageEncoding)
    {
        wprintf(L"Using MTOM encoding.\n");
    }

    if (FAILED(ParseTransport(clientURL, &clientTransport, &clientSecurityMode)))
    {
        wprintf(L"Illegal protocol for communicating with the client service.\n");
        return -1;
    }
    if (HTTP_TRANSPORT != clientTransport)
    {
        // Communication from the command line to the client service is restricted to HTTP
        // for simplicity reasons. The client service can accept any protocol and encoding
        // so the protocol can easily be replaced in code here.
        // The communication from client to server service supports all encodings and protocols
        // and this command line lets you specify all of them.
        wprintf(L"Illegal protocol for communicating with the client service.\n");
        return -1;
    }

    // Currently the requests sent to client service are unsecured. The actual file transfer can be secured.
    if (NO_SECURITY != clientSecurityMode)
    {
        wprintf(L"Illegal security mode for communicating with the client service.\n");
        return -1;
    }

    if (FAILED(ParseTransport(serverURL, &serverTransport, &serverSecurityMode)))
    {
        wprintf(L"Illegal protocol for communication between client and server service.\n");
        return -1;
    }

    if (NO_SECURITY == serverSecurityMode)
    {
        wprintf(L"Using unsecured protocol for communication between client and server service.\n");
    }
    
    // Create an error object for storing rich error information.
    IfFailedExit(WsCreateError(NULL, 0, &error));

    // Transferring a file could take a long time. Thus we use a long timeout of one hour.
    // If the user does not want to wait he or she should use the async mode.
    ULONG timeout = 1000*60*60*24;
    WS_CHANNEL_PROPERTY channelProperty[2];
    channelProperty[0].id = WS_CHANNEL_PROPERTY_RECEIVE_RESPONSE_TIMEOUT;
    channelProperty[0].value = &timeout;
    channelProperty[0].valueSize = sizeof(timeout);

    channelProperty[1].id = WS_CHANNEL_PROPERTY_RECEIVE_TIMEOUT;
    channelProperty[1].value = &timeout;
    channelProperty[1].valueSize = sizeof(timeout);

    IfFailedExit(WsCreateChannel(WS_CHANNEL_TYPE_REQUEST, 
        WS_HTTP_CHANNEL_BINDING, channelProperty, 2, NULL, &channel, error));
    
    IfFailedExit(WsCreateMessageForChannel(channel, NULL, 0, &requestMessage, error));   
    IfFailedExit(WsCreateMessageForChannel(channel, NULL, 0, &replyMessage, error));
   
    // Initialize address of service
    address.url.chars = clientURL;
    IfFailedExit(SizeTToULong(wcslen(address.url.chars), &address.url.length));
    
    IfFailedExit(WsOpenChannel(channel, &address, NULL, error));    

    if (synchronous)
    {
        userRequest.requestType = SYNC_REQUEST;
    }
    else
    {
        userRequest.requestType = ASYNC_REQUEST;
    }

    userRequest.serverUri = serverURL;
    userRequest.serverProtocol = serverTransport;
    userRequest.securityMode = serverSecurityMode;
    userRequest.messageEncoding = messageEncoding;
    userRequest.sourcePath = sourcePath;
    userRequest.destinationPath = destinationPath;

    IfFailedExit(WsCreateHeap(65536, 0, NULL, 0, &heap, NULL));

    WS_MESSAGE_DESCRIPTION userRequestMessageDescription;
    userRequestMessageDescription.action = &userRequestAction;
    userRequestMessageDescription.bodyElementDescription = &userRequestElement;

    WS_MESSAGE_DESCRIPTION userResponseMessageDescription;
    userResponseMessageDescription.action = &userResponseAction;
    userResponseMessageDescription.bodyElementDescription = &userResponseElement;
      
    hr = WsRequestReply(
        channel, 
        requestMessage, 
        &userRequestMessageDescription,
        WS_WRITE_REQUIRED_VALUE,
        &userRequest, 
        sizeof(userRequest), 
        replyMessage, 
        &userResponseMessageDescription, 
        WS_READ_REQUIRED_POINTER, 
        heap, 
        &userResponse, 
        sizeof(userResponse), 
        NULL, 
        error);

    if (WS_E_ENDPOINT_FAULT_RECEIVED == hr)
    {
        // We got a fault.
        WS_FAULT* fault;
        IfFailedExit(WsGetFaultErrorProperty(error, WS_FAULT_ERROR_PROPERTY_FAULT, &fault, sizeof(fault)));
        wprintf(L"The server returned a fault:\n");

        for (ULONG i = 0; i < fault->reasonCount; i++)
        {
            if (fault->reasons[i].text.length < 1024)
            {
                wprintf(L"%.*s\n", fault->reasons[i].text.length, fault->reasons[i].text.chars);
            }
            else
            {
                wprintf(L"Fault reason too long to display.\n");
            }
        }           
    } 
    else if (SUCCEEDED(hr))
    {
        if (TRANSFER_ASYNC == userResponse->returnValue)
        {
            wprintf(L"The request is completing asynchronously.\n");
        }
        else if (TRANSFER_SUCCESS == userResponse->returnValue)
        {
            wprintf(L"The request succeeded.\n");
        }
        else
        {
            wprintf(L"Unexpected return value from server.\n");
        }
    }
        
    EXIT
   
    if (FAILED(hr) && WS_E_ENDPOINT_FAULT_RECEIVED != hr)
    {
        wprintf(L"Unexpected failure:\n");
        PrintError(hr, error);
    }
    
    if (requestMessage != NULL)
    {
        WsFreeMessage(requestMessage);
    }
    if (replyMessage != NULL)
    {
        WsFreeMessage(replyMessage);
    }

    CleanupChannel(channel);

    if (error != NULL)
    {
        WsFreeError(error);
    }

    if (NULL != heap)
    {
        WsFreeHeap(heap);
    }

    return SUCCEEDED(hr) ? 0 : -1;
}


