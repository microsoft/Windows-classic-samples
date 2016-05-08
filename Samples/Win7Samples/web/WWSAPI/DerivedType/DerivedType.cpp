
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
#include "DerivedType.xsd.h"

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

void PrintPayloadType(
    __in_opt PayloadBaseType* payloadType)
{
    if (payloadType == NULL)
    {
        return;
    }

    Payload1Type* payload1Type = payloadType->AsPayload1Type();
    if (payload1Type == NULL)
    {
        // This is a base type
        printf("base type\n");
        printf("Id %d\n", payloadType->Id);
        
    }
    else
    {
        // this is derived type
        printf("derived type\n");
        printf("Id %d\n", payloadType->Id);
        printf("BoolValue %d\n", payload1Type->BoolValue);
        wprintf(
            L"StringValue %s\n", 
            payload1Type->StringValue);
    }
}


// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_XML_BUFFER* xmlBuffer = NULL;
    WS_HEAP* heap = NULL;
    WS_XML_READER* xmlReader = NULL;
    WS_XML_WRITER* xmlWriter = NULL;
    PayloadBaseType* baseType = NULL;
    Payload1Type* payload1Type = NULL;
    
    static const WS_XML_STRING dataElement = WS_XML_STRING_VALUE("data");
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
    
    // Create a heap to store deserialized data
    hr = WsCreateHeap(
        /*maxSize*/ 4096, 
        /*trimSize*/ 512, 
        NULL, 
        0, 
        &heap, 
        error);
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
    // Create an XML buffer on the specified heap
    hr = WsCreateXmlBuffer(
        heap, 
        NULL, 
        0, 
        &xmlBuffer, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Set the writer to output to the XML buffer
    hr = WsSetOutputToBuffer(
        xmlWriter, 
        xmlBuffer, 
        NULL, 
        0, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create a wrapper element for the two embedded elements
    hr = WsWriteStartElement(
        xmlWriter,
        NULL,
        &dataElement,
        &emptyNamespace,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    baseType = new PayloadBaseType();
    if (baseType == NULL)
    {
        goto Exit;
    }
    baseType->Id = 1;
    
    payload1Type = new Payload1Type();
    if (payload1Type == NULL)
    {
        goto Exit;
    }
    payload1Type->Id = 2;
    payload1Type->BoolValue = FALSE;
    payload1Type->StringValue = L"hello world";
    // Write the base type using the element description of the base type.
    // An xsi:type attribute will be added to the XML document for the element
    // indicating this is the base type.
    hr = WsWriteElement(
        xmlWriter,
        &DerivedType_xsd.globalElements.PayloadBase, 
        WS_WRITE_REQUIRED_VALUE,
        baseType, 
        sizeof(PayloadBaseType), 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Write the derived type using the element description of the base type.
    // An xsi:type attribute will be added to the XML document for the element
    // indicating this is the derived type.
    hr = WsWriteElement(
        xmlWriter,
        &DerivedType_xsd.globalElements.PayloadBase, 
        WS_WRITE_REQUIRED_VALUE,
        payload1Type, 
        sizeof(Payload1Type), 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsWriteEndElement(
        xmlWriter,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Flush writer so all XML content is put in the buffer
    hr = WsFlushWriter(xmlWriter, 0, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Set the reader input to current position of XML buffer
    hr = WsSetInputToBuffer(xmlReader, xmlBuffer, NULL, 0, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Read pass the wrapper element
    hr = WsReadToStartElement(
        xmlReader,
        &dataElement,
        &emptyNamespace,
        NULL,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
        
    
    hr = WsReadStartElement(
        xmlReader,
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    PayloadBaseType* outBaseType = NULL;
    
    // Read the first element using element description for the base
    // type. The type of returning structure is that of that base type.
    hr = WsReadElement(
        xmlReader,
        &DerivedType_xsd.globalElements.PayloadBase, 
        WS_READ_REQUIRED_POINTER,
        heap,
        &outBaseType,
        sizeof(PayloadBaseType*),
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    PrintPayloadType(outBaseType);
    
    // Read the second element using element description for the base
    // type. The type of returning structure is that of the derived type.
    hr = WsReadElement(
        xmlReader,
        &DerivedType_xsd.globalElements.PayloadBase, 
        WS_READ_REQUIRED_POINTER,
        heap,
        &outBaseType,
        sizeof(PayloadBaseType*),
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    PrintPayloadType(outBaseType);
    
    hr = WsReadEndElement(
        xmlReader,
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
    
    delete baseType;
    delete payload1Type;
    
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
    if (heap != NULL)
    {
        WsFreeHeap(heap);
    }
    
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
