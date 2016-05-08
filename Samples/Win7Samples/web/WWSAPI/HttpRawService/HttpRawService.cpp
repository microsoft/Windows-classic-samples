//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"

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

HANDLE closeServer = NULL;  

static const WS_XML_STRING verbHeaderName = WS_XML_STRING_VALUE("Verb");
static const WS_XML_STRING contentTypeHeaderName = WS_XML_STRING_VALUE("Content-Type");

static const WS_XML_STRING statusCodeName = WS_XML_STRING_VALUE("StatusCode");
static const WS_XML_STRING statusTextName = WS_XML_STRING_VALUE("StatusText");

static const WS_STRING replyContentType = WS_STRING_VALUE(L"text/html");
static const BYTE replyBodyBytes[] = "<html><body>Hello World</body><html>";

static const WS_ELEMENT_DESCRIPTION bytesBodyDescription = 
{ 
    NULL,
    NULL,
    WS_BYTES_TYPE,
    NULL
};

static const WS_MESSAGE_DESCRIPTION bytesMessageDescription = 
{ 
    NULL,
    const_cast<WS_ELEMENT_DESCRIPTION*>(&bytesBodyDescription)
};

static const WS_MESSAGE_DESCRIPTION emptyMessageDescription = 
{ 
    NULL,
    NULL
};

static const WS_HTTP_HEADER_MAPPING contentTypeHeaderMapping =
{
    WS_XML_STRING_VALUE("Content-Type"),
    0,
};

static const WS_HTTP_HEADER_MAPPING* const responseHeaderMappings[] =
{
    &contentTypeHeaderMapping,
};

static const WS_HTTP_HEADER_MAPPING* const requestHeaderMappings[] =
{
    &contentTypeHeaderMapping,
};

static const WS_HTTP_MESSAGE_MAPPING messageMapping = 
{ 
    WS_HTTP_REQUEST_MAPPING_VERB, 
    WS_HTTP_RESPONSE_MAPPING_STATUS_TEXT | WS_HTTP_RESPONSE_MAPPING_STATUS_CODE,
    const_cast<WS_HTTP_HEADER_MAPPING**>(requestHeaderMappings),
    WsCountOf(requestHeaderMappings), 
    const_cast<WS_HTTP_HEADER_MAPPING**>(responseHeaderMappings),
    WsCountOf(responseHeaderMappings), 
};

HRESULT SetStatus(
    __in WS_MESSAGE* replyMessage, 
    __in ULONG statusCode, 
    __in_z WCHAR* statusText,
    __in_opt WS_ERROR* error)
{
    HRESULT hr;

    // Add the status text to the message
    hr = WsAddMappedHeader(
        replyMessage,
        &statusTextName,
        WS_WSZ_TYPE,
        WS_WRITE_REQUIRED_POINTER,
        &statusText,
        sizeof(statusText),
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }

    // Add the status code to the message
    hr = WsAddMappedHeader(
        replyMessage,
        &statusCodeName,
        WS_UINT32_TYPE,
        WS_WRITE_REQUIRED_VALUE,
        &statusCode,
        sizeof(statusCode),
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }

Exit:
    return hr;
}

HRESULT SendFailureMessage(
    __in WS_CHANNEL* channel, 
    __in WS_MESSAGE* replyMessage, 
    __in ULONG statusCode, 
    __in_z WCHAR* statusText, 
    __in_opt WS_ERROR* error)
{
    HRESULT hr;

    // Add status code/text to the reply
    hr = SetStatus(
        replyMessage,
        statusCode,
        statusText,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }

    // Send the reply
    hr = WsSendMessage(
        channel, 
        replyMessage, 
        &emptyMessageDescription, 
        WS_WRITE_REQUIRED_VALUE,
        NULL,
        0,
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }

Exit:
    return hr;
}

HRESULT CALLBACK ProcessMessage(
    __in const WS_OPERATION_CONTEXT* context, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);
    
    WS_CHANNEL* channel = NULL;
    HRESULT hr = S_OK;
    WS_MESSAGE* requestMessage = NULL;
    WS_MESSAGE* replyMessage = NULL;
    WS_HEAP* heap;
    
    // Get the request mesasge
    hr = WsGetOperationContextProperty(
        context, 
        WS_OPERATION_CONTEXT_PROPERTY_INPUT_MESSAGE, 
        &requestMessage, 
        sizeof(requestMessage), 
        error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    // Get the channel 
    hr = WsGetOperationContextProperty(
        context, 
        WS_OPERATION_CONTEXT_PROPERTY_CHANNEL, 
        &channel, 
        sizeof(channel), 
        error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    // Get the heap 
    hr = WsGetOperationContextProperty(
        context, 
        WS_OPERATION_CONTEXT_PROPERTY_HEAP, 
        &heap, 
        sizeof(heap), 
        error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    // Create a reply message
    hr = WsCreateMessageForChannel(
        channel,
        NULL, 
        0, 
        &replyMessage, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Initialize reply message
    hr = WsInitializeMessage(replyMessage, WS_REPLY_MESSAGE, requestMessage, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
        
    // Get the HTTP Verb
    WS_STRING verb;
    hr = WsGetMappedHeader(
        requestMessage, 
        &verbHeaderName, 
        WS_SINGLETON_HEADER,
        0,
        WS_STRING_TYPE,
        WS_READ_REQUIRED_VALUE, 
        NULL, 
        &verb, 
        sizeof(verb), 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    wprintf(L"Verb: %.*s\n", verb.length, verb.chars);
    
    // Check the verb
    if (CompareString(
        LOCALE_INVARIANT, 
        NORM_IGNORECASE, 
        verb.chars, 
        verb.length, 
        L"POST", 
        4) == CSTR_EQUAL)
    {
        // It is a POST, so read the body
    
        // Get the content type
        WS_STRING contentType;
        hr = WsGetMappedHeader(
            requestMessage, 
            &contentTypeHeaderMapping.headerName, 
            WS_SINGLETON_HEADER,
            0,
            WS_STRING_TYPE,
            WS_READ_REQUIRED_VALUE, 
            NULL, 
            &contentType, 
            sizeof(contentType), 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        wprintf(L"Content-Type: %.*s\n", contentType.length, contentType.chars);
    
        // Read the bytes of the message body
        WS_BYTES body;
        hr = WsReadBody(
            requestMessage,
            &bytesBodyDescription,
            WS_READ_REQUIRED_VALUE,
            heap,
            &body,
            sizeof(body),
            error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
        wprintf(L"%.*S\n", body.length, body.bytes);
    }
    else if (CompareString(
            LOCALE_INVARIANT, 
            NORM_IGNORECASE, 
            verb.chars, 
            verb.length, 
            L"GET", 
            3) == CSTR_EQUAL)
    {
        // It is a GET, so the message body is empty
    }
    else
    {
        // Unrecognized verb
        hr = SendFailureMessage(
            channel, 
            replyMessage, 
            405,
            L"Method Not Allowed", 
            error);
        goto Exit;
    }
    
    // Read end of request message
    hr = WsReadMessageEnd(
        channel, 
        requestMessage, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Set content type header of reply
    hr = WsAddMappedHeader(
        replyMessage,
        &contentTypeHeaderMapping.headerName, 
        WS_STRING_TYPE,
        WS_WRITE_REQUIRED_VALUE,
        &replyContentType,
        sizeof(replyContentType),
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Set status of reply
    hr = SetStatus(replyMessage, 200, L"OK", error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Set up the bytes to return in the HTTP body
    WS_BYTES body;
    body.bytes = const_cast<BYTE*>(replyBodyBytes);
    body.length = sizeof(replyBodyBytes);
    
    // Send the reply message
    hr = WsSendMessage(
        channel, 
        replyMessage, 
        &bytesMessageDescription, 
        WS_WRITE_REQUIRED_VALUE,
        &body,
        sizeof(body),
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
Exit:
    if (replyMessage != NULL)
    {
        WsFreeMessage(
            replyMessage);
    }
    return hr;    
}

HRESULT CALLBACK CloseChannelCallback(
    __in const WS_OPERATION_CONTEXT* context, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext)
{
UNREFERENCED_PARAMETER(context);
UNREFERENCED_PARAMETER(asyncContext);

    static volatile ULONG messageCount = 0;

    if (InterlockedIncrement(&messageCount) == 2)
    {
    SetEvent(closeServer);
    }
    return S_OK;
}

static const WS_SERVICE_CONTRACT messageContract = 
{ 
    NULL, 
    ProcessMessage, 
    NULL
};

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_SERVICE_HOST* host = NULL;
    WS_SERVICE_ENDPOINT serviceEndpoint = {};
    const WS_SERVICE_ENDPOINT* serviceEndpoints[1];
    WS_ERROR* error = NULL;
    
    serviceEndpoints[0] = &serviceEndpoint;
    
    // Specify that the HTTP channel should surface non-SOAP messages
    WS_CHANNEL_PROPERTY channelPropertyArray[2];
    WS_ENCODING rawEncoding = WS_ENCODING_RAW;
    channelPropertyArray[0].id = WS_CHANNEL_PROPERTY_ENCODING;
    channelPropertyArray[0].value = &rawEncoding;
    channelPropertyArray[0].valueSize = sizeof(rawEncoding);
    
    // Specify how HTTP requests and responses are mapped to the message object
    channelPropertyArray[1].id = WS_CHANNEL_PROPERTY_HTTP_MESSAGE_MAPPING;
    channelPropertyArray[1].value = const_cast<WS_HTTP_MESSAGE_MAPPING*>(&messageMapping);
    channelPropertyArray[1].valueSize = sizeof(messageMapping);
    
    WS_SERVICE_ENDPOINT_PROPERTY serviceEndpointPropertyArray[1]; 
    WS_SERVICE_PROPERTY_CLOSE_CALLBACK closeCallbackProperty = {CloseChannelCallback};
    serviceEndpointPropertyArray[0].id = WS_SERVICE_ENDPOINT_PROPERTY_CLOSE_CHANNEL_CALLBACK;
    serviceEndpointPropertyArray[0].value = &closeCallbackProperty;
    serviceEndpointPropertyArray[0].valueSize = sizeof(closeCallbackProperty);
    
    
    // Initialize service endpoint
    serviceEndpoint.address.url.chars = L"http://+:80/example"; // address given as uri
    serviceEndpoint.address.url.length = (ULONG)wcslen(serviceEndpoint.address.url.chars);
    serviceEndpoint.channelBinding = WS_HTTP_CHANNEL_BINDING; // channel binding for the endpoint
    serviceEndpoint.channelType = WS_CHANNEL_TYPE_REPLY; // the channel type
    serviceEndpoint.contract = &messageContract;  // the contract
    serviceEndpoint.properties = serviceEndpointPropertyArray;
    serviceEndpoint.propertyCount = WsCountOf(serviceEndpointPropertyArray);
    serviceEndpoint.channelProperties.properties = channelPropertyArray; // Channel properties
    serviceEndpoint.channelProperties.propertyCount = WsCountOf(channelPropertyArray); // Channel property Count
    serviceEndpoint.channelProperties.properties = channelPropertyArray;
    serviceEndpoint.channelProperties.propertyCount = WsCountOf(channelPropertyArray);
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // Create Event object for closing the server
    closeServer = CreateEvent(
        NULL, 
        TRUE, 
        FALSE, 
        NULL);
    if (closeServer == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }   
    // Creating a service host
    hr = WsCreateServiceHost(
        serviceEndpoints, 
        1, 
        NULL, 
        0, 
        &host, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // WsOpenServiceHost to start the listeners in the service host 
    hr = WsOpenServiceHost(
        host, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    WaitForSingleObject(closeServer, INFINITE);
    // Close the service host
    hr = WsCloseServiceHost(host, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    if (host != NULL)
    {
        WsFreeServiceHost(host);
    }
    
    
    if (error != NULL)
    {
        WsFreeError(error);
    }
    if (closeServer != NULL)
    {
        CloseHandle(closeServer);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
