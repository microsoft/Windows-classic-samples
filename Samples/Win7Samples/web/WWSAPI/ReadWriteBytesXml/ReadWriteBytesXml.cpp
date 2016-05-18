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

static WS_BYTES data1 = { 18, (BYTE*)"binary data inline" };
static WS_BYTES data2 = { 34, (BYTE*)"binary data in mime part, buffered" };
static WS_BYTES data3 = { 39, (BYTE*)"binary data in mime part, via PushBytes" };
static WS_BYTES data4 = { 39, (BYTE*)"binary data in mime part, via PullBytes" };

HRESULT CALLBACK PushCallback(
    __in void* callbackState, 
    __in WS_WRITE_CALLBACK writeCallback, 
    __in void* writeCallbackState, 
    __in const WS_ASYNC_CONTEXT* asyncContext, 
    __in WS_ERROR* error)
{
    WS_BYTES* buffer = (WS_BYTES*)callbackState;
    return writeCallback(writeCallbackState, buffer, 1, asyncContext, error);
}

HRESULT CALLBACK PullCallback(
    __in void* callbackState, 
    __out_bcount_part(maxSize, *actualSize) void* buffer, 
    __in ULONG maxSize, 
    __out ULONG* actualSize, 
    __in const WS_ASYNC_CONTEXT* asyncContext, 
    __in WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);
    UNREFERENCED_PARAMETER(error);

    WS_BYTES* bytes = (WS_BYTES*)callbackState;
    ULONG size = min(bytes->length, maxSize);
    ::CopyMemory(buffer, bytes->bytes, size);
    bytes->bytes += size;
    bytes->length -= size;

    (*actualSize) = size;
    return S_OK;
}

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_XML_WRITER* xmlWriter = NULL;
    WS_XML_READER* xmlReader = NULL;
    
    static const WS_STRING boundary = WS_STRING_VALUE(L"boundaryString");
    static const WS_STRING startInfo = WS_STRING_VALUE(L"startInfo");
    static const WS_STRING startUri = WS_STRING_VALUE(L"http://tempuri.org");
    
    static const WS_XML_STRING dataElement = WS_XML_STRING_VALUE("data");
    static const WS_XML_STRING bytesElement = WS_XML_STRING_VALUE("bytes");
    static const WS_XML_STRING emptyNamespace = WS_XML_STRING_VALUE("");
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    WS_XML_WRITER_BUFFER_OUTPUT streamOutput;
    ZeroMemory(&streamOutput, sizeof(streamOutput));
    streamOutput.output.outputType = WS_XML_WRITER_OUTPUT_TYPE_BUFFER;
    
    WS_XML_WRITER_TEXT_ENCODING textWriterEncoding;
    ZeroMemory(&textWriterEncoding, sizeof(textWriterEncoding));
    textWriterEncoding.encoding.encodingType = WS_XML_WRITER_ENCODING_TYPE_TEXT;
    textWriterEncoding.charSet = WS_CHARSET_UTF8;
    
    WS_XML_WRITER_MTOM_ENCODING mtomWriterEncoding;
    ZeroMemory(&mtomWriterEncoding, sizeof(mtomWriterEncoding));
    mtomWriterEncoding.encoding.encodingType = WS_XML_WRITER_ENCODING_TYPE_MTOM;
    mtomWriterEncoding.textEncoding = &textWriterEncoding.encoding;
    mtomWriterEncoding.writeMimeHeader = TRUE;
    mtomWriterEncoding.boundary = boundary;
    mtomWriterEncoding.startInfo = startInfo;
    mtomWriterEncoding.startUri = startUri;
    mtomWriterEncoding.maxInlineByteCount = data1.length;
    
    // Create an XML writer
    hr = WsCreateWriter(
        NULL, 
        0, 
        &xmlWriter, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsSetOutput(xmlWriter, &mtomWriterEncoding.encoding, &streamOutput.output, NULL, 0, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsWriteStartElement(xmlWriter, NULL, &dataElement, &emptyNamespace, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    for (ULONG type = 0; type < 4; type++)
    {
        hr = WsWriteStartElement(xmlWriter, NULL, &bytesElement, &emptyNamespace, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        switch (type)
        {
            case 0:
            {
                // Write the bytes into the element (sizeof(data) <= maxInlineByteCount)
                hr = WsWriteBytes(xmlWriter, data1.bytes, data1.length, error);
                if (FAILED(hr))
                {
                    goto Exit;
                }
                break;
            }
            case 1:
            {
                // Write the bytes into a MIME part (sizeof(data) > maxInlineByteCount) with a copy
                hr = WsWriteBytes(xmlWriter, data2.bytes, data2.length, error);
                if (FAILED(hr))
                {
                    goto Exit;
                }
                break;
            }
            case 2:
                // Push the bytes into a MIME part.  In buffered mode, this is no more efficient than
                // WsWriteBytes, but in streamed, it avoids a copy.
                hr = WsPushBytes(xmlWriter, PushCallback, &data3, error);
                if (FAILED(hr))
                {
                    goto Exit;
                }
                break;
            case 3:
                // Pull the bytes into a MIME part.  In streamed mode, this is no more efficient than
                // WsWriteBytes, but in buffered, it avoids a copy.
                hr = WsPullBytes(xmlWriter, PullCallback, &data4, error);
                if (FAILED(hr))
                {
                    goto Exit;
                }
                break;
        }
    
        hr = WsWriteEndElement(xmlWriter, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
    hr = WsWriteEndElement(xmlWriter, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsFlushWriter(xmlWriter, 0, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    WS_BYTES buffer;
    hr = WsGetWriterProperty(xmlWriter, WS_XML_WRITER_PROPERTY_BYTES, &buffer, sizeof(buffer), error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    for (ULONG i = 0; i < buffer.length; i++)
    {
        printf("%c", buffer.bytes[i]);
    }
    
    WS_XML_READER_BUFFER_INPUT bufferInput;
    ZeroMemory(&bufferInput, sizeof(bufferInput));
    bufferInput.input.inputType = WS_XML_READER_INPUT_TYPE_BUFFER;
    bufferInput.encodedData = buffer.bytes;
    bufferInput.encodedDataSize = buffer.length;
    
    WS_XML_READER_TEXT_ENCODING textReaderEncoding;
    ZeroMemory(&textReaderEncoding, sizeof(textReaderEncoding));
    textReaderEncoding.encoding.encodingType = WS_XML_READER_ENCODING_TYPE_TEXT;
    textReaderEncoding.charSet = WS_CHARSET_AUTO;
    
    WS_XML_READER_MTOM_ENCODING mtomReaderEncoding;
    ZeroMemory(&mtomReaderEncoding, sizeof(mtomReaderEncoding));
    mtomReaderEncoding.encoding.encodingType = WS_XML_READER_ENCODING_TYPE_MTOM;
    mtomReaderEncoding.textEncoding = &textReaderEncoding.encoding;
    mtomReaderEncoding.readMimeHeader = TRUE;
    
    // Create an XML reader
    hr = WsCreateReader(NULL, 0, &xmlReader, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsSetInput(xmlReader, &mtomReaderEncoding.encoding, &bufferInput.input, NULL, 0, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsReadToStartElement(xmlReader, &dataElement, &emptyNamespace, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsReadStartElement(xmlReader, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    for (ULONG type = 0; type < 4; type++)
    {
        hr = WsReadToStartElement(xmlReader, &bytesElement, &emptyNamespace, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsReadStartElement(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        printf("data%d: \"", type + 1);
        for (;;)
        {
            BYTE buffer[128];
            ULONG byteCount;
            hr = WsReadBytes(xmlReader, buffer, sizeof(buffer), &byteCount, error);
            if (FAILED(hr))
            {
                goto Exit;
            }
            if (byteCount == 0)
            {
                break;
            }
            for (ULONG i = 0; i < byteCount; i++)
            {
                printf("%c", buffer[i]);
            }
        }
        printf("\"\n");
    
        hr = WsReadEndElement(xmlReader, error);
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
    if (xmlWriter != NULL)
    {
        WsFreeWriter(xmlWriter);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
