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

// Main entry point
int __cdecl wmain(
    __in int argc, 
    __in_ecount(argc) wchar_t **argv)
{
    HRESULT hr = S_OK;
    
    WS_ERROR* error = NULL;
    WS_HEAP* heap = NULL;
    WS_XML_BUFFER* buffer = NULL;
    WS_XML_WRITER* writer = NULL;
    WS_XML_READER* reader = NULL;
    void* newXml = NULL;
    ULONG newXmlLength = 0;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create a heap to store deserialized data
    hr = WsCreateHeap(
        /*maxSize*/ 2048, 
        /*trimSize*/ 512, 
        NULL, 
        0, 
        &heap, 
        error);
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
    
    BOOL readFromBytes = FALSE;
    BOOL writeToBytes = FALSE;
    for (int i = 1; i < argc; i++)
    {
        if (wcscmp(
            argv[i], 
            L"/readFromBytes") == 0)
        {
            readFromBytes = TRUE;
        }
        if (wcscmp(
            argv[i],
            L"/writeToBytes") == 0)
        {
            writeToBytes = TRUE;
        }
    }
    
    // Some xml to read and write
    static const char xml[] = "<a><b>1</b><c>2</c></a>";
    
    if (!readFromBytes)
    {
        wprintf(
            L"Reading using WsReadXmlBuffer\n");
    
        // Setup the reader input source
        WS_XML_READER_BUFFER_INPUT bufferInput;
        ZeroMemory(
            &bufferInput, 
            sizeof(bufferInput));
        bufferInput.input.inputType = WS_XML_READER_INPUT_TYPE_BUFFER;
        bufferInput.encodedData = (void*)xml;
        bufferInput.encodedDataSize = (ULONG)strlen(xml);
    
        WS_XML_READER_TEXT_ENCODING textEncoding;
        ZeroMemory(
            &textEncoding, 
            sizeof(textEncoding));
    
        textEncoding.encoding.encodingType = WS_XML_READER_ENCODING_TYPE_TEXT;
        textEncoding.charSet = WS_CHARSET_AUTO;
    
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
    
        // Read the xml into a WS_XML_BUFFER
        hr = WsReadXmlBuffer(
            reader, 
            heap, 
            &buffer, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    else
    {
        wprintf(
            L"Reading using WsReadXmlBufferFromBytes\n");
    
        // Shortcut for the above code
        hr = WsReadXmlBufferFromBytes(
            reader, 
            NULL, 
            NULL, 
            0, 
            xml, 
            (ULONG)strlen(xml), 
            heap, 
            &buffer, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
    if (!writeToBytes)
    {
        wprintf(
            L"Writing using WsWriteXmlBuffer\n");
    
        WS_XML_WRITER_BUFFER_OUTPUT bufferOutput;
        ZeroMemory(
            &bufferOutput, 
            sizeof(bufferOutput));
        bufferOutput.output.outputType = WS_XML_WRITER_OUTPUT_TYPE_BUFFER;
    
        WS_XML_WRITER_TEXT_ENCODING textEncoding;
        ZeroMemory(
            &textEncoding, 
            sizeof(textEncoding));
        
        textEncoding.encoding.encodingType = WS_XML_WRITER_ENCODING_TYPE_TEXT ;
        textEncoding.charSet = WS_CHARSET_UTF8;
    
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
    
        // Write the XML buffer to the writer
        hr = WsWriteXmlBuffer(
            writer, 
            buffer, 
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
    
        printf(
            "%.*s\n", 
            bytes.length, 
            (char*)bytes.bytes);
    }
    else
    {
        wprintf(
            L"Writing using WsWriteXmlBufferToBytes\n");
    
        hr = WsWriteXmlBufferToBytes(
            writer, 
            buffer, 
            NULL, 
            NULL, 
            0, 
            heap, 
            &newXml, 
            &newXmlLength, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        printf(
            "%.*s\n", newXmlLength, 
            (char*)newXml);
    }
    
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
    if (reader != NULL)
    {
        WsFreeReader(reader);
    }
    if (heap != NULL)
    {
        WsFreeHeap(heap);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
