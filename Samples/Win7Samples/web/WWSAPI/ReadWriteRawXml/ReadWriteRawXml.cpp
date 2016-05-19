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
    WS_XML_WRITER* xmlWriter = NULL;
    WS_XML_READER* xmlReader = NULL;
    
    // Command line parameter specifies whether to read or write raw
    BOOL readRaw = FALSE;
    BOOL writeRaw = FALSE;
    if (argc == 2)
    {
        wchar_t* arg = argv[1];
        if (wcscmp(arg, L"none") == 0)
        {
        }
        else if (wcscmp(arg, L"read") == 0)
        {
            readRaw = TRUE;
        }
        else if (wcscmp(arg, L"write") == 0)
        {
            writeRaw = TRUE;
        }
        else if (wcscmp(arg, L"both") == 0)
        {
            readRaw = TRUE;
            writeRaw = TRUE;
        }
        else
        {
            wprintf(L"Invalid command line argument '%s'\n", arg);
            return 1;
        }
    }
    else
    {
        wprintf(L"usage : WsReadWriteRawXml.exe [none|read|write|both]\n");
        return 1;
    }
    
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
    
    if (readRaw)
    {
        // Setup some non-textual xml to read
        static const char bytes[] = "{ 1, 2, 3 }";
        static const WS_XML_READER_RAW_ENCODING rawEncoding = 
        { 
            { WS_XML_READER_ENCODING_TYPE_RAW } 
        };
        static const WS_XML_READER_BUFFER_INPUT bufferInput =
        { 
            { WS_XML_READER_INPUT_TYPE_BUFFER },
            (void*)bytes,
            sizeof(bytes) - 1,
        };
        hr = WsSetInput(
            xmlReader, 
            &rawEncoding.encoding, 
            &bufferInput.input, 
            NULL, 
            0, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    else
    {
        // Setup the base64 encoded version of "{ 1, 2, 3 }";
        static const char bytes[] = "eyAxLCAyLCAzIH0=";
        WS_XML_READER_TEXT_ENCODING textEncoding = 
        { 
            { WS_XML_READER_ENCODING_TYPE_TEXT },
            WS_CHARSET_AUTO
        };
        WS_XML_READER_BUFFER_INPUT bufferInput =
        { 
            { WS_XML_READER_INPUT_TYPE_BUFFER },
            (void*)bytes,
            sizeof(bytes) - 1,
        };
        // Since the base64 encoded data is at the root in the raw encoding, we need
        // to set the allow fragment property
        BOOL allowFragment = TRUE;
        WS_XML_READER_PROPERTY properties[1];
        properties[0].id = WS_XML_READER_PROPERTY_ALLOW_FRAGMENT;
        properties[0].value = &allowFragment;
        properties[0].valueSize = sizeof(allowFragment);
    
        hr = WsSetInput(
            xmlReader, 
            &textEncoding.encoding, 
            &bufferInput.input, 
            properties,
            WsCountOf(properties),
            error);
    
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
    if (writeRaw)
    {
        // Setup the writer to emit the raw data
        WS_XML_WRITER_RAW_ENCODING rawEncoding = 
        { 
            { WS_XML_WRITER_ENCODING_TYPE_RAW } 
        };
        WS_XML_WRITER_BUFFER_OUTPUT bufferOutput =
        { 
            { WS_XML_WRITER_OUTPUT_TYPE_BUFFER }
        };
        hr = WsSetOutput(
            xmlWriter, 
            &rawEncoding.encoding, 
            &bufferOutput.output, 
            NULL, 
            0, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    else
    {
        // Setup the writer to emit the data as base64 characters
        WS_XML_WRITER_TEXT_ENCODING textEncoding = 
        { 
            { WS_XML_WRITER_ENCODING_TYPE_TEXT },
            WS_CHARSET_UTF8
        };
        WS_XML_WRITER_BUFFER_OUTPUT bufferOutput =
        { 
            { WS_XML_WRITER_OUTPUT_TYPE_BUFFER }
        };
        // Since the base64 encoded data is at the root in the raw encoding, we need
        // to set the allow fragment property
        BOOL allowFragment = TRUE;
        WS_XML_WRITER_PROPERTY properties[1];
        properties[0].id = WS_XML_WRITER_PROPERTY_ALLOW_FRAGMENT;
        properties[0].value = &allowFragment;
        properties[0].valueSize = sizeof(allowFragment);
    
        hr = WsSetOutput(
            xmlWriter, 
            &textEncoding.encoding, 
            &bufferOutput.output, 
            properties, 
            WsCountOf(properties),
            error);
    
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
    hr = WsCopyNode(xmlWriter, xmlReader, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    WS_BYTES bytes;
    hr = WsGetWriterProperty(
        xmlWriter, 
        WS_XML_WRITER_PROPERTY_BYTES, 
        &bytes, 
        sizeof(bytes), 
        error);
    
    if (FAILED(hr))
    {
        goto Exit;
    }
    printf("%.*s\n", bytes.length, bytes.bytes);
    
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
