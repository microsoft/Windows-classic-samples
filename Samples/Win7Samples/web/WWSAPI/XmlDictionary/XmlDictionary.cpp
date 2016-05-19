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

struct ColorDictionary
{
    WS_XML_DICTIONARY dictionary;
    WS_XML_STRING red;
    WS_XML_STRING yellow;
    WS_XML_STRING blue;
    WS_XML_STRING green;
};

ColorDictionary colorDictionary =
{
    { 
        { /* fd7d93f6-a9ec-40a5-a492-8bb45b1e3a5f */
            0xfd7d93f6,
            0xa9ec,
            0x40a5,
            {0xa4, 0x92, 0x8b, 0xb4, 0x5b, 0x1e, 0x3a, 0x5f}
        },
        &colorDictionary.red,
        4, 
        TRUE
    },
    WS_XML_STRING_DICTIONARY_VALUE("red", &colorDictionary.dictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("yellow", &colorDictionary.dictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("blue", &colorDictionary.dictionary, 2),
    WS_XML_STRING_DICTIONARY_VALUE("green", &colorDictionary.dictionary, 3),
};


struct ShapeDictionary
{
    WS_XML_DICTIONARY dictionary;
    WS_XML_STRING circle;
    WS_XML_STRING square;
    WS_XML_STRING triangle;
};

ShapeDictionary shapeDictionary =
{
    { 
        { /* 4eab536f-d3a9-418c-b6d4-26b2b926eafe */
            0x4eab536f,
            0xd3a9,
            0x418c,
            {0xb6, 0xd4, 0x26, 0xb2, 0xb9, 0x26, 0xea, 0xfe}
        },
        &colorDictionary.red,
        3, 
        TRUE
    },
    WS_XML_STRING_DICTIONARY_VALUE("circle", &shapeDictionary.dictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("square", &shapeDictionary.dictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("triangle", &shapeDictionary.dictionary, 2),
};


struct ObjectsDictionary
{
    WS_XML_DICTIONARY dictionary;
    WS_XML_STRING objects;
    WS_XML_STRING color;
    WS_XML_STRING ns;
};

ObjectsDictionary objectsDictionary =
{
    { 
        { /* 34065de6-b672-417f-96dc-c4436a055bf1 */
            0x34065de6,
            0xb672,
            0x417f,
            {0x96, 0xdc, 0xc4, 0x43, 0x6a, 0x05, 0x5b, 0xf1}
        },
        &objectsDictionary.objects,
        3,
        TRUE
    },
    WS_XML_STRING_DICTIONARY_VALUE("objects", &objectsDictionary.dictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("color", &objectsDictionary.dictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("ns", &objectsDictionary.dictionary, 2),
};


struct MergedDictionary
{
    WS_XML_DICTIONARY dictionary;
    WS_XML_STRING strings[32];
    ULONG stringCount;
};

MergedDictionary mergedDictionary =
{
    { 
        { /* 472007e3-7378-4ac3-846d-bf8c63550a73 */
            0x472007e3,
            0x7378,
            0x4ac3,
            {0x84, 0x6d, 0xbf, 0x8c, 0x63, 0x55, 0x0a, 0x73}
        },
        mergedDictionary.strings,
        0,
        FALSE
    },
};

HRESULT CALLBACK DynamicStringCallback(
    __in void* callbackState, 
    __in const WS_XML_STRING* value, 
    __out BOOL* found, 
    __out ULONG* id, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackState);
    UNREFERENCED_PARAMETER(error);

    // Only merge strings that are const
    if (value->dictionary != NULL && value->dictionary->isConst)
    {
        // See if we've seen this string before
        for (ULONG i = 0; i < mergedDictionary.dictionary.stringCount; i++)
        {
            if (value->length == mergedDictionary.strings[i].length &&
                memcmp(value->bytes, mergedDictionary.strings[i].bytes, value->length) == 0)
            {
                (*found) = TRUE;
                (*id) = mergedDictionary.strings[i].id;
                return S_OK;
            }
        }
        if (mergedDictionary.dictionary.stringCount < WsCountOf(mergedDictionary.strings))
        {
            // Add this string to the merged dictionary
            ULONG index = mergedDictionary.dictionary.stringCount;
            mergedDictionary.strings[index].bytes = value->bytes;
            mergedDictionary.strings[index].length = value->length;
            mergedDictionary.strings[index].dictionary = &mergedDictionary.dictionary;
            mergedDictionary.strings[index].id = index;
            mergedDictionary.dictionary.stringCount++;
            (*id) = index;
            (*found) = TRUE;
            return S_OK;
        }
    }
    (*found) = FALSE;
    return S_OK;
}

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_XML_WRITER* writer = NULL;
    WS_XML_READER* reader = NULL;
    WS_HEAP* heap = NULL;
    
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
    
    // Setup the output
    WS_XML_WRITER_BUFFER_OUTPUT bufferOutput;
    ZeroMemory(&bufferOutput, sizeof(bufferOutput));
    bufferOutput.output.outputType = WS_XML_WRITER_OUTPUT_TYPE_BUFFER;
    
    // Setup the encoding
    WS_XML_WRITER_BINARY_ENCODING writerEncoding;
    ZeroMemory(&writerEncoding, sizeof(writerEncoding));
    writerEncoding.encoding.encodingType = WS_XML_WRITER_ENCODING_TYPE_BINARY;
    writerEncoding.staticDictionary = &objectsDictionary.dictionary;
    writerEncoding.dynamicStringCallback = DynamicStringCallback;
    writerEncoding.dynamicStringCallbackState = NULL;
    
    // Setup the writer
    hr = WsSetOutput(
        writer, 
        &writerEncoding.encoding, 
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
        &objectsDictionary.objects, 
        &objectsDictionary.ns, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Write some xml using strings from all the dictionaries
    static const WS_XML_STRING* shapes[3] = { &shapeDictionary.triangle, &shapeDictionary.square, &shapeDictionary.circle };
    static const WS_XML_STRING* colors[3] = { &colorDictionary.green, &colorDictionary.blue, &colorDictionary.red };
    for (ULONG i = 0; i < 3; i++)
    {
        hr = WsWriteStartElement(
            writer, 
            NULL, 
            shapes[i], 
            &objectsDictionary.ns, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsWriteStartAttribute(
            writer, 
            NULL, 
            &objectsDictionary.color, 
            &objectsDictionary.ns, 
            FALSE, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsWriteType(
            writer, 
            WS_ATTRIBUTE_TYPE_MAPPING, 
            WS_XML_STRING_TYPE, NULL, 
            WS_WRITE_REQUIRED_VALUE, 
            colors[i], 
            sizeof(*colors[i]), 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsWriteEndAttribute(
            writer, 
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
    ZeroMemory(&bufferInput, sizeof(bufferInput));
    bufferInput.input.inputType = WS_XML_READER_INPUT_TYPE_BUFFER;
    bufferInput.encodedData = bytes.bytes;
    bufferInput.encodedDataSize = bytes.length;
    
    // Setup the encoding
    WS_XML_READER_BINARY_ENCODING readerEncoding;
    ZeroMemory(
        &readerEncoding, 
        sizeof(readerEncoding));
    
    readerEncoding.encoding.encodingType = WS_XML_READER_ENCODING_TYPE_BINARY;
    readerEncoding.staticDictionary = &objectsDictionary.dictionary;
    readerEncoding.dynamicDictionary = &mergedDictionary.dictionary;
    
    // Setup the reader
    hr = WsSetInput(
        reader, 
        &readerEncoding.encoding, 
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
        &objectsDictionary.objects, 
        &objectsDictionary.ns, 
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
        BOOL found;
        hr = WsReadToStartElement(
            reader, 
            NULL, 
            NULL, 
            &found, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        if (!found)
        {
            break;
        }
    
        const WS_XML_NODE* node;
        hr = WsGetReaderNode(
            reader, 
            &node, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        const WS_XML_ELEMENT_NODE* elementNode = (WS_XML_ELEMENT_NODE*)node;
        printf("%.*s: ", elementNode->localName->length, elementNode->localName->bytes);
    
        ULONG index;
        hr = WsFindAttribute(
            reader, 
            &objectsDictionary.color, 
            &objectsDictionary.ns, 
            TRUE, 
            &index, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsReadStartAttribute(
            reader, 
            index, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        WS_XML_STRING color;
        hr = WsReadType(
            reader, 
            WS_ATTRIBUTE_TYPE_MAPPING, 
            WS_XML_STRING_TYPE, 
            NULL, 
            WS_READ_REQUIRED_VALUE, 
            heap, 
            &color, 
            sizeof(color), 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        printf(
            "%.*s\n", 
            color.length, 
            color.bytes);
    
        hr = WsReadEndAttribute(
            reader, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        hr = WsSkipNode(
            reader, 
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
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
