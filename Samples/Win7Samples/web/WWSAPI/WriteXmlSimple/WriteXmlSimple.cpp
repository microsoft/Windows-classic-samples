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

WS_XML_STRING valueLocalName = WS_XML_STRING_VALUE("value");
WS_XML_STRING valueNs = WS_XML_STRING_VALUE("");

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_XML_WRITER* writer = NULL;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create an XML writer
    hr = WsCreateWriter(
        NULL, 
        0, 
        &writer, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Setup the output
    WS_XML_WRITER_BUFFER_OUTPUT bufferOutput;
    ZeroMemory(
        &bufferOutput, 
        sizeof(bufferOutput));
    
    bufferOutput.output.outputType = WS_XML_WRITER_OUTPUT_TYPE_BUFFER;
    
    // Setup the encoding
    WS_XML_WRITER_TEXT_ENCODING textEncoding;
    ZeroMemory(
        &textEncoding, 
        sizeof(textEncoding));
    
    textEncoding.encoding.encodingType = WS_XML_WRITER_ENCODING_TYPE_TEXT;
    textEncoding.charSet = WS_CHARSET_UTF16LE;
    
    // Setup the writer
    hr = WsSetOutput(
        writer, 
        &textEncoding.encoding, 
        &bufferOutput.output, 
        NULL, 
        0, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = WsWriteStartElement(
        writer, 
        NULL, 
        &valueLocalName, 
        &valueNs, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = WsWriteChars(
        writer, 
        L"Hello world.", 
        12, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = WsWriteEndElement(
        writer, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    WS_BYTES bytes;
    hr = WsGetWriterProperty(
        writer, 
        WS_XML_WRITER_PROPERTY_BYTES, 
        &bytes, 
        sizeof(bytes), 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    wprintf(
        L"%.*s\n", 
        bytes.length / sizeof(WCHAR), 
        (WCHAR*)bytes.bytes);
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    
    if (writer != NULL)
    {
        WsFreeWriter(writer);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
