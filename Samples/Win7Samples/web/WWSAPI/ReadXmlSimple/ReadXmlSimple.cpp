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

static const WS_XML_STRING valueLocalName = WS_XML_STRING_VALUE("value");
static const WS_XML_STRING valueNs = WS_XML_STRING_VALUE("");

static const char xml[] = 
"<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
"<value>Hello World.</value>";

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_XML_READER* reader = NULL;
    
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
        &reader, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Setup the input
    WS_XML_READER_BUFFER_INPUT bufferInput;
    ZeroMemory(
        &bufferInput,
        sizeof(bufferInput));
    bufferInput.input.inputType = WS_XML_READER_INPUT_TYPE_BUFFER;
    bufferInput.encodedData = (BYTE*)xml;
    bufferInput.encodedDataSize = sizeof(xml) - 1;
    
    // Setup the encoding
    WS_XML_READER_TEXT_ENCODING textEncoding;
    ZeroMemory(
        &textEncoding,
        sizeof(textEncoding));
    textEncoding.encoding.encodingType = WS_XML_READER_ENCODING_TYPE_TEXT;
    textEncoding.charSet = WS_CHARSET_AUTO;
    
    
    // Setup the reader
    hr = WsSetInput(
        reader, 
        &textEncoding.encoding, 
        &bufferInput.input, 
        NULL, 
        0, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsReadToStartElement(
        reader, 
        &valueLocalName, 
        &valueNs, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsReadStartElement(
        reader,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    for (;;)
    {
            WCHAR chars[128];
            ULONG charCount;
            hr = WsReadChars(
                reader,
                chars,
                128, 
                &charCount,
                error);
        if (FAILED(hr))
        {
            goto Exit;
        }
            if (charCount == 0)
            {
                break;
            }
            wprintf(L"%.*s", charCount, chars);
    }
    wprintf(L"\n");
    hr = WsReadEndElement(
        reader,
        error);
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
    
    if (reader != NULL)
    {
        WsFreeReader(reader);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
