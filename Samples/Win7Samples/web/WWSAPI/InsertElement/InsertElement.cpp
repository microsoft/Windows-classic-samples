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
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_HEAP* heap = NULL;
    WS_XML_BUFFER* buffer = NULL;
    WS_XML_WRITER* writer = NULL;
    WS_XML_READER* reader = NULL;
    void* xml = NULL;
    ULONG xmlLength = 0;
    WS_XML_NODE_POSITION securityEndElementPosition;
    
    static const WS_XML_STRING soapNs = WS_XML_STRING_VALUE("http://schemas.xmlsoap.org/soap/envelope/");
    static const WS_XML_STRING wsseNs = WS_XML_STRING_VALUE("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd");
    static const WS_XML_STRING envelope = WS_XML_STRING_VALUE("Envelope");
    static const WS_XML_STRING header = WS_XML_STRING_VALUE("Header");
    static const WS_XML_STRING security = WS_XML_STRING_VALUE("Security");
    static const WS_XML_STRING dsNs = WS_XML_STRING_VALUE("http://www.w3.org/2000/09/xmldsig#");
    static const WS_XML_STRING signature = WS_XML_STRING_VALUE("Signature");
    
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
    // Create an XML buffer on the specified heap
    hr = WsCreateXmlBuffer(
        heap, 
        NULL, 
        0, 
        &buffer, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Set the writer to output to the XML buffer
    hr = WsSetOutputToBuffer(
        writer, 
        buffer, 
        NULL, 
        0, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Write the envelope element
    hr = WsWriteStartElement(writer, NULL, &envelope, &soapNs, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Write the header element
    hr = WsWriteStartElement(writer, NULL, &header, &soapNs, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Write the security element
    hr = WsWriteStartElement(writer, NULL, &security, &wsseNs, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Force the security element start tag to be written so the position obtained
    // is "after" the security element
    hr = WsWriteEndStartElement(writer, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsGetWriterPosition(writer, &securityEndElementPosition, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Close the security element
    hr = WsWriteEndElement(writer, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Close the header element
    hr = WsWriteEndElement(writer, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Close the envelope element
    hr = WsWriteEndElement(writer, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Move the write back
    hr = WsSetWriterPosition(writer, &securityEndElementPosition, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Write the "signature" element
    hr = WsWriteStartElement(writer, NULL, &signature, &dsNs, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Close the signature element
    hr = WsWriteEndElement(writer, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Generate the bytes of the document
    ULONG indent = 4;
    WS_XML_WRITER_PROPERTY properties[1];
    properties[0].id = WS_XML_WRITER_PROPERTY_INDENT;
    properties[0].value = &indent;
    properties[0].valueSize = sizeof(indent);
    hr = WsWriteXmlBufferToBytes(writer, buffer, NULL, properties, WsCountOf(properties), heap, &xml, &xmlLength, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    printf("%.*s\n", xmlLength, (char*)xml);
    
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
