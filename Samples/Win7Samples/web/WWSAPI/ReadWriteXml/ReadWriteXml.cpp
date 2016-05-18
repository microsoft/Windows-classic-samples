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
#include "PurchaseOrder.wsdl.h"

struct PurchaseOrderDictionary
{
    WS_XML_DICTIONARY dictionary;
    WS_XML_STRING quantity;
    WS_XML_STRING productName;
    WS_XML_STRING purchaseOrderType;
    WS_XML_STRING purchaseOrderNamespace;
};

static const PurchaseOrderDictionary purchaseOrderDictionary =
{
    { 
        { 0x89d3da8b, 0xec46, 0x4fe8, { 0x9a, 0xb4, 0x48, 0x62, 0xb2, 0x69, 0x71, 0x8a } },
        (WS_XML_STRING*)&purchaseOrderDictionary.quantity,
        4, 
        TRUE 
    },
    WS_XML_STRING_DICTIONARY_VALUE("quantity", &purchaseOrderDictionary.dictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("productName", &purchaseOrderDictionary.dictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("PurchaseOrderType", &purchaseOrderDictionary.dictionary, 2),
    WS_XML_STRING_DICTIONARY_VALUE("http://example.com", &purchaseOrderDictionary.dictionary, 3),
};


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
    WS_XML_BUFFER* xmlBuffer = NULL;
    WS_XML_WRITER* xmlWriter = NULL;
    WS_XML_READER* xmlReader = NULL;
    static const WCHAR* productName = L"Pencil";
    
    // Command line parameter specifies whether to use typed or untyped api
    BOOL typed;
    if (argc == 2)
    {
        wchar_t* arg = argv[1];
        if (wcscmp(arg, L"typed") == 0)
        {
            typed = TRUE;
        }
        else if (wcscmp(arg, L"untyped") == 0)
        {
            typed = FALSE;
        }
        else
        {
            wprintf(L"Invalid command line argument '%s'\n", arg);
            return 1;
        }
    }
    else
    {
        wprintf(L"usage : WsReadWriteXml.exe [typed|untyped]\n");
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
    
    // Write xml into the buffer
    if (typed)
    {
        // Initialize body data
        _PurchaseOrderType purchaseOrder;
        purchaseOrder.quantity = 1;
        purchaseOrder.productName = (WCHAR*)productName;
    
        // Write purchase order
        hr = WsWriteElement(
            xmlWriter, 
            &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
            WS_WRITE_REQUIRED_VALUE,
            &purchaseOrder, 
            sizeof(purchaseOrder), 
            error);
    
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    else
    {
        // Write purchase order start element
        hr = WsWriteStartElement(xmlWriter, NULL, &purchaseOrderDictionary.purchaseOrderType, &purchaseOrderDictionary.purchaseOrderNamespace, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Write quantity element
        int quantity = 1;
        hr = WsWriteStartElement(xmlWriter, NULL, &purchaseOrderDictionary.quantity, &purchaseOrderDictionary.purchaseOrderNamespace, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsWriteValue(xmlWriter, WS_INT32_VALUE_TYPE, &quantity, sizeof(quantity), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsWriteEndElement(xmlWriter, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Write product name start element
        hr = WsWriteStartElement(xmlWriter, NULL, &purchaseOrderDictionary.productName, &purchaseOrderDictionary.purchaseOrderNamespace, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Write product name
        hr = WsWriteChars(xmlWriter, productName, (ULONG)wcslen(productName), error);
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
    
        // Write purchase order end element
        hr = WsWriteEndElement(xmlWriter, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
    // Flush writer so all XML content is put in the buffer
    hr = WsFlushWriter(xmlWriter, 0, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create an XML reader
    hr = WsCreateReader(NULL, 0, &xmlReader, error);
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
    
    if (typed)
    {
        // Read purchase order into heap
        _PurchaseOrderType* purchaseOrder;
        hr = WsReadElement(xmlReader, &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
            WS_READ_REQUIRED_POINTER, heap, &purchaseOrder, sizeof(purchaseOrder), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Print out purchase order contents
        wprintf(L"%ld, %s\n", 
            purchaseOrder->quantity, 
            purchaseOrder->productName);
    
        // Done with purchase order
        WsResetHeap(heap, NULL);
    }
    else
    {
        // Read to purchase order element
        hr = WsReadToStartElement(xmlReader, &purchaseOrderDictionary.purchaseOrderType, &purchaseOrderDictionary.purchaseOrderNamespace, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Read purchase order element
        hr = WsReadStartElement(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Read to quantity element
        hr = WsReadToStartElement(xmlReader, &purchaseOrderDictionary.quantity, &purchaseOrderDictionary.purchaseOrderNamespace, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Read quantity element as integer
        long quantity;
        hr = WsReadStartElement(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsReadValue(xmlReader, WS_INT32_VALUE_TYPE, &quantity, sizeof(quantity), error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        hr = WsReadEndElement(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Read to product name element
        hr = WsReadToStartElement(xmlReader, &purchaseOrderDictionary.productName, &purchaseOrderDictionary.purchaseOrderNamespace, NULL, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Read product name start element
        hr = WsReadStartElement(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
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
                hr = WS_E_INVALID_FORMAT;
                goto Exit;
            }
        }
    
        // Zero terminate product name string
        productName[length] = L'\0';
    
        // Print out purchase order contents
        wprintf(L"%ld, %s\n", 
            quantity, 
            productName);
    
        // Read product name end element
        hr = WsReadEndElement(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Read purchase order end element
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
