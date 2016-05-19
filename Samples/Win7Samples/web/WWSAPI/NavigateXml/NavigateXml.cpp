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
static const WS_XML_STRING productNameLocalName = WS_XML_STRING_VALUE("ProductName");
static const WS_XML_STRING purchaseOrderLocalName = WS_XML_STRING_VALUE("PurchaseOrder");
static const WS_XML_STRING purchaseOrderNamespace = WS_XML_STRING_VALUE("http://example.com");

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_HEAP* heap = NULL;
    WS_XML_WRITER* xmlWriter = NULL;
    WS_XML_BUFFER* xmlBuffer = NULL;
    WS_XML_READER* xmlReader = NULL;
    static const WS_STRING productNames[4] = { 
        WS_STRING_VALUE(L"Pencil"),
        WS_STRING_VALUE(L"Ruler"),
        WS_STRING_VALUE(L"Eraser"),
        WS_STRING_VALUE(L"Pen") 
    };
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = WsCreateHeap(/* maxSize */ 65536, /* trimSize */ 4096, NULL, 0, &heap, error);
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
    
    // Write purchase order start element
    hr = WsWriteStartElement(xmlWriter, NULL, &purchaseOrderLocalName, &purchaseOrderNamespace, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    for (ULONG i = 0; i < 4; i++)
    {
        // Write product name start element
        hr = WsWriteStartElement(xmlWriter, NULL, &productNameLocalName, &purchaseOrderNamespace, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Write product name
        hr = WsWriteChars(xmlWriter, productNames[i].chars, productNames[i].length, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Write product name end element
        hr = WsWriteEndElement(xmlWriter, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
    // Write purchase order end element
    hr = WsWriteEndElement(xmlWriter, error);
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
    
    // Set the reader input to current position of XML buffer
    hr = WsSetInputToBuffer(xmlReader, xmlBuffer, NULL, 0, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Navigate to the purchase order
    hr = WsMoveReader(xmlReader, WS_MOVE_TO_ROOT_ELEMENT, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Navigate to the first child element
    hr = WsMoveReader(xmlReader, WS_MOVE_TO_CHILD_ELEMENT, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Record the position of each product
    WS_XML_NODE_POSITION productPositions[16];
    ULONG productCount = 0;
    for (;;)
    {
        BOOL found;
    
        // Read to product name element
        hr = WsReadToStartElement(xmlReader, &productNameLocalName, &purchaseOrderNamespace, &found, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        if (!found)
        {
            break;
        }
    
        if (productCount >= WsCountOf(productPositions))
        {
            hr = E_FAIL;
            goto Exit;
        }
    
        // Record product position
        hr = WsGetReaderPosition(xmlReader, &productPositions[productCount], error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        productCount++;
    
        // Skip element
        hr = WsSkipNode(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
    // Move to each product in reverse and print out the contents
    for (ULONG i = productCount; i > 0; i--)
    {
        hr = WsSetReaderPosition(xmlReader, &productPositions[i - 1], error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsReadStartElement(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
    #pragma warning(disable:26015)
        // Read product name into buffer
        WCHAR productName[100];
        ULONG length = 0;
        ULONG available = WsCountOf(productName) - 1; // leave space for terminating '\0'
        for (;;)
        {
            // Read next block of chars
            ULONG charsRead;
            hr = WsReadChars(xmlReader, &productName[length], available, &charsRead, error);
            if (FAILED(hr))
            {
                goto Exit;
            }
    
            if (charsRead == 0)
            {
                // No more chars
                break;
            }
    
            length += charsRead;
            available -= charsRead;
    
            if (available == 0)
            {
                hr = E_FAIL;
                goto Exit;
            }
        }
    
        // Zero terminate product name string
        productName[length] = L'\0';
    
        // Print out purchase order contents
        wprintf(L"%s\n", 
            productName);
    
        hr = WsReadEndElement(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    
    if (heap != NULL)
    {
        WsFreeHeap(heap);
    }
    if (xmlWriter != NULL)
    {
        WsFreeWriter(xmlWriter);
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
