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

static const WS_XML_STRING orders = WS_XML_STRING_VALUE("Orders");
static const WS_XML_STRING purchaseOrder = WS_XML_STRING_VALUE("PurchaseOrder");
static const WS_XML_STRING id = WS_XML_STRING_VALUE("id");
static const WS_XML_STRING nameSpace = WS_XML_STRING_VALUE("http://example.com");
static const WS_XML_STRING emptyNamespace = WS_XML_STRING_VALUE("");

static const char* xml = 
"<Orders xmlns='http://example.com'>"
    "<PurchaseOrder id='1001'>"
    "</PurchaseOrder>"
    "<PurchaseOrder id='1002'>"
    "</PurchaseOrder>"
    "<PurchaseOrder id='1003'>"
    "</PurchaseOrder>"
"</Orders>";

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_XML_READER* xmlReader = NULL;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create an XML reader
    hr = WsCreateReader(
        NULL,
        0, 
        &xmlReader, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Setup the source input
    WS_XML_READER_BUFFER_INPUT bufferInput;
    ZeroMemory(&bufferInput, sizeof(bufferInput));
    bufferInput.input.inputType = WS_XML_READER_INPUT_TYPE_BUFFER;
    bufferInput.encodedData = (BYTE*)xml;
    bufferInput.encodedDataSize = (ULONG)strlen(xml);
    
    // Setup the source encoding
    WS_XML_READER_TEXT_ENCODING textEncoding;
    ZeroMemory(&textEncoding, sizeof(textEncoding));
    textEncoding.encoding.encodingType = WS_XML_READER_ENCODING_TYPE_TEXT;
    textEncoding.charSet = WS_CHARSET_AUTO;
    
    // Setup the reader
    hr = WsSetInput(xmlReader, &textEncoding.encoding, &bufferInput.input, NULL, 0, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = WsReadToStartElement(xmlReader, &orders, &nameSpace, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = WsReadStartElement(xmlReader, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    for (;;)
    {
        BOOL found;
        hr = WsReadToStartElement(xmlReader, &purchaseOrder, &nameSpace, &found, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        if (!found)
        {
            break;
        }
        // The attribute we're looking for is from the empty namespace
        ULONG index;
        hr = WsFindAttribute(xmlReader, &id, &emptyNamespace, TRUE, &index, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        hr = WsReadStartAttribute(xmlReader, index, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        __int32 value;
        hr = WsReadValue(xmlReader, WS_INT32_VALUE_TYPE, &value, sizeof(value), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        printf("Id='%d'\n", value);
        hr = WsReadEndAttribute(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        hr = WsSkipNode(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    hr = WsReadEndElement(xmlReader, error);
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
    
    if (xmlReader != NULL)
    {
        WsFreeReader(xmlReader);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
